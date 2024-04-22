/*
 * opencog/atoms/sensory/IRChatStream.cc
 *
 * Copyright (C) 2020 Linas Vepstas
 * All Rights Reserved
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License v3 as
 * published by the Free Software Foundation and including the exceptions
 * at http://opencog.org/wiki/Licenses
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public License
 * along with this program; if not, write to:
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */


#include <opencog/util/exceptions.h>
#include <opencog/util/oc_assert.h>
#include <opencog/atoms/base/Node.h>
#include <opencog/atoms/value/StringValue.h>
#include <opencog/atoms/value/ValueFactory.h>

#include <opencog/atoms/sensory-types/sensory_types.h>
#include "IRChatStream.h"

#include "IRC.h"

#include <errno.h>
#include <string.h> // for strerror()

using namespace opencog;

IRChatStream::IRChatStream(Type t, const std::string& str)
	: OutputStream(t)
{
	OC_ASSERT(nameserver().isA(_type, I_R_CHAT_STREAM),
		"Bad IRChatStream constructor!");
	init(str);
}

IRChatStream::IRChatStream(const std::string& str)
	: OutputStream(I_R_CHAT_STREAM)
{
	init(str);
}

IRChatStream::IRChatStream(const Handle& senso)
	: OutputStream(I_R_CHAT_STREAM)
{
	if (SENSORY_NODE != senso->get_type())
		throw RuntimeException(TRACE_INFO,
			"Expecting SensoryNode, got %s\n", senso->to_string().c_str());

	init(senso->get_name());
}

IRChatStream::~IRChatStream()
{
	if (_conn)
	{
		_conn->disconnect();
		delete _conn;
	}
}

// ==================================================================
// thunks

int IRChatStream::xend_of_motd(const char* params, irc_reply_data* ird,
                               void* data)
{
	IRC* conn = static_cast<IRC*>(data);
	IRChatStream* that = static_cast<IRChatStream*>(conn->context);
	return that->end_of_motd(params, ird);
}

int IRChatStream::xgot_privmsg(const char* params, irc_reply_data* ird,
                               void* data)
{
	IRC* conn = static_cast<IRC*>(data);
	IRChatStream* that = static_cast<IRChatStream*>(conn->context);
	return that->got_privmsg(params, ird);
}

int IRChatStream::xgot_kick(const char* params, irc_reply_data* ird,
                               void* data)
{
	IRC* conn = static_cast<IRC*>(data);
	IRChatStream* that = static_cast<IRChatStream*>(conn->context);
	return that->got_kick(params, ird);
}

// ==================================================================

/* printf can puke if these fields are NULL */
static void fixup_reply(irc_reply_data* ird)
{
	ird->nick = (NULL == ird->nick) ? (char *) "" : ird->nick;
	ird->ident = (NULL == ird->ident) ? (char *) "" : ird->ident;
	ird->host = (NULL == ird->host) ? (char *) "" : ird->host;
	ird->target = (NULL == ird->target) ? (char *) "" : ird->target;
}

// Do not attempt to join any channels, until MOTD has arrived.
int IRChatStream::end_of_motd(const char* params, irc_reply_data* ird)
{
	fixup_reply(ird);
	printf("chatbot got params=%s\n", params);
	printf("chatbot got motd nick=%s ident=%s host=%s target=%s\n",
		ird->nick, ird->ident, ird->host, ird->target);

	int rc = _conn->join(_channel.c_str());
	if (rc)
		throw RuntimeException(TRACE_INFO,
			"Unable to join channel (%d) to URL \"%s\"\n", rc, _uri.c_str());

	return 0;
}

// ==================================================================

int IRChatStream::got_privmsg(const char* params, irc_reply_data* ird)
{
	fixup_reply(ird);
	printf("chatbot priv params=%s\n", params);
	printf("chatbot priv motd nick=%s ident=%s host=%s target=%s\n",
		ird->nick, ird->ident, ird->host, ird->target);

	return 0;
}

int IRChatStream::got_kick(const char* params, irc_reply_data* ird)
{
	return 0;
}

// ==================================================================
void IRChatStream::looper(void)
{
	printf("duuud enter looper\n");
	_conn->message_loop();
}

/// IRC URL format is described here:
/// ???
/// The URL format is described in
/// https://en.wikipedia.org/wiki/IRC
/// and we adhere to that.
///
/// URI formats are:
/// irc://<host>[:<port>]/[<channel>[?<channel_keyword>]]
/// ircs://<host>[:<port>]/[<channel>[?<channel_keyword>]]
/// irc6://<host>[:<port>]/[<channel>[?<channel_keyword>]]
///
void IRChatStream::init(const std::string& url)
{
	_conn = nullptr;

	if (0 != url.compare(0, 6, "irc://"))
		throw RuntimeException(TRACE_INFO,
			"Unsupported URL \"%s\"\n", url.c_str());

	// Make a copy, for debuggingg purposes.
	_uri = url;

	// Ignore the first 6 chars "irc://"
	size_t base = 6;
	size_t sls = url.find('/', base);
	size_t col = url.find(':', base);
	if (std::string::npos == sls)
		throw RuntimeException(TRACE_INFO,
			"Invalid IRC URL \"%s\"\n", url.c_str());

	std::string host;
	int port = 6667;
	if (std::string::npos == col or sls < col)
	{
		host = url.substr(base, sls-base);
	}
	else
	{
		// const char *ho = url.substr(base, col-base).c_str();
		host = url.substr(base, col-base);
		port = atoi(url.substr(col+1, sls-col-1).c_str());
	}
	_channel = url.substr(sls+1);

	// Defaults
	const char* nick = "sensor";
	const char* user = "botski";
	const char* name = "Atomese Sensory Node";
	const char* pass = "";

	_conn = new IRC;
	_conn->context = this;

	_conn->hook_irc_command("376", &xend_of_motd);
	_conn->hook_irc_command("PRIVMSG", &xgot_privmsg);
	_conn->hook_irc_command("KICK", &xgot_kick);

	int rc = _conn->start(host.c_str(), port, nick, user, name, pass);
	if (rc)
		throw RuntimeException(TRACE_INFO,
			"Unable to connect (%d) to URL \"%s\"\n", rc, url.c_str());

	// Run I/O loop in it's own thread.
	_loop = new std::thread(&IRChatStream::looper, this);
}

// ==============================================================

// This will read one line from the file stream, and return that line.
// So, a line-oriented, buffered interface. For now.
void IRChatStream::update() const
{
	if (nullptr == _conn) { _value.clear(); return; }

#if 0
#define BUFSZ 4080
	char buff[BUFSZ];
	char* rd = fgets(buff, BUFSZ, _fh);
	if (nullptr == rd)
	{
		fclose(_fh);
		_fh = nullptr;
		_value.clear();
		return;
	}

	_value.resize(1);
	_value[0] = createNode(ITEM_NODE, buff);
#endif
}

// ==============================================================
// Write stuff to a file.
void IRChatStream::prt_value(const ValuePtr& content)
{
#if 0
	if (content->is_type(STRING_VALUE))
	{
		StringValuePtr svp(StringValueCast(content));
		const std::vector<std::string>& strs = svp->value();
		for (const std::string& str : strs)
			fprintf(_fh, "%s", str.c_str());
		return;
	}
	if (content->is_type(NODE))
	{
		const std::string& name = HandleCast(content)->get_name();
		fprintf(_fh, " %s", name.c_str());
		return;
	}
	if (content->is_type(LINK_VALUE))
	{
		LinkValuePtr lvp(LinkValueCast(content));
		const ValueSeq& vals = lvp->value();
		for (const ValuePtr& v : vals)
			prt_value(v);
		return;
	}

	// Backwards-compat: AllowListLink and SetLink (only!?)
	// Why restrict? I dunno. Seems like the right thing to do.
	Type tc = content->get_type();
	if (LIST_LINK == tc or SET_LINK == tc)
	{
		const HandleSeq& oset = HandleCast(content)->getOutgoingSet();
		for (const Handle& h : oset)
			prt_value(h);
		return;
	}

	throw RuntimeException(TRACE_INFO,
		"Expecting strings, got %s\n", content->to_string().c_str());
#endif
}

// Write stuff to a file.
ValuePtr IRChatStream::write_out(AtomSpace* as, bool silent,
                                   const Handle& cref)
{
	ValuePtr content = cref;
#if 0
	if (nullptr == _fh)
		throw RuntimeException(TRACE_INFO,
			"Text stream not open: URI \"%s\"\n", _uri.c_str());

	if (cref->is_executable())
	{
		content = cref->execute(as, silent);
		if (nullptr == content)
			throw RuntimeException(TRACE_INFO,
				"Expecting something to write from %s\n",
				cref->to_string().c_str());
	}

	// If it is not a stream, then just print and return.
	if (not content->is_type(LINK_STREAM_VALUE))
	{
		prt_value(content);
		fflush(_fh);
		return content;
	}

	// If it is a stream, enter infinite loop, until it is exhausted.
	LinkValuePtr lvp(LinkValueCast(content));
	while (true)
	{
		const ValueSeq& vals = lvp->value();

		// If the stream is returning an empty list, assume we
		// are done. Exit the loop.
		if (0 == vals.size()) break;

		// A different case arises if the stream keeps returning
		// empty LinkValues. This is kind of pathological, and
		// arguably, its a bug upstream somewhere, but for now,
		// we catch this and handle it.
		size_t nprinted = 0;
		for (const ValuePtr& v : vals)
		{
			if (v->is_type(LINK_VALUE) and 0 == v->size()) continue;
			prt_value(v);
			nprinted ++;
		}
		fflush(_fh);
		if (0 == nprinted) break;
	}
#endif
	return content;
}

// ==============================================================

bool IRChatStream::operator==(const Value& other) const
{
	// Derived classes use this, so use get_type()
	if (get_type() != other.get_type()) return false;

	if (this == &other) return true;

	return LinkValue::operator==(other);
}

// ==============================================================

// Adds factory when library is loaded.
DEFINE_VALUE_FACTORY(I_R_CHAT_STREAM, createIRChatStream, std::string)
DEFINE_VALUE_FACTORY(I_R_CHAT_STREAM, createIRChatStream, Handle)

// ====================================================================

void opencog_sensory_irc_init(void)
{
	// Force shared lib ctors to run
};
