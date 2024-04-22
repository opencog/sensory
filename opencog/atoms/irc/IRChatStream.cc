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
	if (nullptr == _conn) return;
	_cancel = true;

	// We can send a quit, but then we never actually
	// wait for the quit reply. So .. whatever.
	_conn->quit("Adios");
	_conn->disconnect();

	_loop->join();
	delete _loop;
}

// ==================================================================
/// The URL format is described in
/// https://en.wikipedia.org/wiki/IRC
///
/// According to wikipedia, URI formats are:
/// irc://<host>[:<port>]/[<channel>[?<channel_keyword>]]
/// ircs://<host>[:<port>]/[<channel>[?<channel_keyword>]]
/// irc6://<host>[:<port>]/[<channel>[?<channel_keyword>]]
///
/// IRC URL format is described here:
/// https://datatracker.ietf.org/doc/html/draft-butcher-irc-url
///
/// I'll use the below approximate simplified form:
/// irc://nick[:pass]@host[:port]/#chan
///
/// See also: https://ircv3.net/irc/index.html
///
void IRChatStream::init(const std::string& url)
{
	_conn = nullptr;
	_cancel = false;

	if (0 != url.compare(0, 6, "irc://"))
		throw RuntimeException(TRACE_INFO,
			"Unsupported URL \"%s\"\n", url.c_str());

	// Make a copy, for debuggingg purposes.
	_uri = url;

	// Ignore the first 6 chars "irc://"
	size_t base = 6;
	size_t nck = url.find('@', base);

	if (std::string::npos == nck)
		throw RuntimeException(TRACE_INFO,
			"Invalid IRC URL \"%s\" expecting irc://nick@host[:port]/#channel\n",
			url.c_str());
	_nick = url.substr(base, nck-base);

	// There may or may not be a port number.
	size_t sls = url.find('/', nck);
	size_t col = url.find(':', nck);
	if (std::string::npos == sls)
		throw RuntimeException(TRACE_INFO,
			"Invalid IRC URL \"%s\" expecting irc://nick@host[:port]/#channel\n",
			url.c_str());

	_port = 6667;
	nck ++;
	if (std::string::npos == col or sls < col)
		_host = url.substr(nck, sls-nck);
	else
	{
		_host = url.substr(nck, col-nck);
		_port = atoi(url.substr(col+1, sls-col-1).c_str());
	}

	// Channels must always start with hash mark.
	_channel = url.substr(sls+1);
	if ('#' != _channel[0]) _channel = "#" + _channel;

	_conn = new IRC;
	_conn->context = this;

	// Hooks run in order. Privmsg will be most common.
	_conn->hook_irc_command("PRIVMSG", &xgot_privmsg);
	_conn->hook_irc_command("376", &xend_of_motd);
	_conn->hook_irc_command("KICK", &xgot_kick);
	_conn->hook_irc_command("403", &xgot_misc);

	// Other messages:
	// NOTICE - server notices
	// 001-005 - server info
	// 250-265 - channel info
	// 375 - start of motd
	// 372 - motd
	// 353 - channel members
	// 366 - end of list of channel members
	// 328 - channel title?
	// 403 - channnel not found
	// MODE - read-write modes

	// Run I/O loop in it's own thread.
	_loop = new std::thread(&IRChatStream::looper, this);
}

// ==================================================================

// Infinite loop.
// XXX Needs major redesign. Works for now.
void IRChatStream::looper(void)
{
	// Defaults
	const char* user = "botski";
	const char* name = "Atomese Sensory Stream";
	const char* pass = "";

	// Loop forever. When the IRC network burps and closes our
	// connection, just log in again.
	while (true)
	{
		printf("Joining network=%s port=%d nick=%s user=%s\n",
			_host.c_str(), _port, _nick.c_str(), user);

		int rc = _conn->start(_host.c_str(), _port, _nick.c_str(),
		                      user, name, pass);
		if (rc)
			throw RuntimeException(TRACE_INFO,
				"Unable to connect (%d) to URL \"%s\"\n", rc, _uri.c_str());

		rc = _conn->message_loop();
		if (_cancel) return;

		if (rc) perror("IRChatStream Error: Socket error");

		// For now, assume we got kicked and want to spawn again.
		_conn->disconnect();
		sleep(20);
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

int IRChatStream::xgot_misc(const char* params, irc_reply_data* ird,
                               void* data)
{
	IRC* conn = static_cast<IRC*>(data);
	IRChatStream* that = static_cast<IRChatStream*>(conn->context);
	return that->got_misc(params, ird);
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
#if DEBUG
	fixup_reply(ird);
	printf("chatbot got params=%s\n", params);
	printf("chatbot got motd nick=%s ident=%s host=%s target=%s\n",
		ird->nick, ird->ident, ird->host, ird->target);
#endif

	_conn->join(_channel.c_str());
	return 0;
}

int IRChatStream::got_misc(const char* params, irc_reply_data* ird)
{
	fixup_reply(ird);
	printf("Misc reply: nick=%s ident=%s host=%s target=%s\n",
		ird->nick, ird->ident, ird->host, ird->target);
	printf("Misc params=%s\n", params);

	return 0;
}

// ==================================================================

int IRChatStream::got_privmsg(const char* params, irc_reply_data* ird)
{
	fixup_reply(ird);
printf(">>> IRC msg from %s to %s =%s\n", ird->nick, ird->target, params);

	// Skip over leading colon.
	const char * start = params + 1;

	// ird->nick is who the message is from.
	// ird->target is who the message is to. It typically has one
	// of two values: the channel name, and so a public message, or
	// my nick, in which case its a private message to me.
	std::vector<std::string> msg;
	msg.push_back(ird->nick);
	msg.push_back(ird->target);
	msg.push_back(start);
	ValuePtr svp(createStringValue(msg));
	push(svp); // concurrent_queue<ValutePtr>::push(svp);
	return 0;
}

int IRChatStream::got_kick(const char* params, irc_reply_data* ird)
{
	fixup_reply(ird);
	printf("got kicked -- input=%s\n", params);
	printf("nick=%s ident=%s host=%s target=%s\n",
	       ird->nick, ird->ident, ird->host, ird->target);
	return 0;
}

// ==============================================================

// This will read one line from the file stream, and return that line.
// So, a line-oriented, buffered interface. For now.
void IRChatStream::update() const
{
	if (nullptr == _conn) { _value.clear(); return; }

	// XXX When is it ever closed ???
	if (is_closed() and 0 == concurrent_queue<ValuePtr>::size()) return;

	// Read one at a time. (???)
	// This will hang, untill there's something to read.
	try
	{
		ValuePtr val;
		const_cast<IRChatStream*>(this) -> pop(val);
		_value.resize(1);
		_value[0] = val;
		return;
	}
	catch (typename concurrent_queue<ValuePtr>::Canceled& e)
	{}

	// If we are here, the queue closed up. Should never happen...
}

// ==============================================================
// Write stuff to a file.
void IRChatStream::do_write(const std::string& str)
{
	const char * msg_target = "linas";
	// const char * msg_target = "#opencog";

	_conn->privmsg(msg_target, str.c_str());
}

// XXX Below is identical to TextStream
void IRChatStream::prt_value(const ValuePtr& content)
{
	if (content->is_type(STRING_VALUE))
	{
		StringValuePtr svp(StringValueCast(content));
		const std::vector<std::string>& strs = svp->value();
		for (const std::string& str : strs)
			do_write(str);
		return;
	}
	if (content->is_type(NODE))
	{
		do_write(HandleCast(content)->get_name());
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
}

// Write stuff to a file.
ValuePtr IRChatStream::write_out(AtomSpace* as, bool silent,
                                   const Handle& cref)
{
	if (nullptr == _conn)
		throw RuntimeException(TRACE_INFO,
			"IRC stream not open: URI \"%s\"\n", _uri.c_str());

// XXX FIXME code below is Identical to TextStream
	ValuePtr content = cref;
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
		if (0 == nprinted) break;
	}
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
