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
#include <opencog/atoms/value/LinkValue.h>
#include <opencog/atoms/value/StringValue.h>
#include <opencog/atoms/value/ValueFactory.h>

#include <opencog/sensory/types/atom_types.h>
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
/// See also: https://ircv3.net/irc/index.html
///
void IRChatStream::init(const std::string& url)
{
	_conn = nullptr;
	_cancel = false;

	if (0 != url.compare(0, 6, "irc://"))
		throw RuntimeException(TRACE_INFO,
			"Unsupported URL \"%s\"\n", url.c_str());

	// Make a copy, for debugging purposes.
	_uri = url;

	// Ignore the first 6 chars "irc://"
	size_t base = 6;
	size_t nck = url.find('@', base);

	if (std::string::npos == nck)
		throw RuntimeException(TRACE_INFO,
			"Invalid IRC URL \"%s\" expecting irc://nick@host[:port]\n",
			url.c_str());
	_nick = url.substr(base, nck-base);

	// There may or may not be a port number.
	size_t sls = url.find('/', nck);
	size_t col = url.find(':', nck);

	_port = 6667;
	nck ++;
	if (std::string::npos == col)
	{
		if (std::string::npos == sls)
			_host = url.substr(nck);
		else
			_host = url.substr(nck, sls-nck);
	}
	else
	{
		_host = url.substr(nck, col-nck);
		_port = atoi(url.substr(col+1, sls-col-1).c_str());
	}

	_conn = new IRC;
	_conn->context = this;

	// Hooks run in order. Privmsg will be most common.
	_conn->hook_irc_command("PRIVMSG", &xgot_privmsg);

	// 001 and 002 are server greetings.
	_conn->hook_irc_command("001", &xgot_privmsg);
	_conn->hook_irc_command("002", &xgot_privmsg);
	_conn->hook_irc_command("433", &xgot_privmsg);
	_conn->hook_irc_command("403", &xgot_privmsg);

	// Bypass the message I/O for some of this stuff.
	_conn->hook_irc_command("376", &xend_of_motd);
	_conn->hook_irc_command("KICK", &xgot_kick);

	// Misc debug printers
	_conn->hook_irc_command("JOIN", &xgot_misc);
	_conn->hook_irc_command("NOTICE", &xgot_misc);
	_conn->hook_irc_command("322", &xgot_misc);
	_conn->hook_irc_command("353", &xgot_misc);
	_conn->hook_irc_command("421", &xgot_misc);
	_conn->hook_irc_command("705", &xgot_misc);
	_conn->hook_irc_command("706", &xgot_misc);

	// Other messages:
	// JOIN - channel join messages
	// NOTICE - server notices
	// 001-005 - server info
	// 250-265 - channel info
	// 321 - start of channel listing
	// 322 - channel name and info
	// 323 - end of channel list
	// 328 - channel title?
	// 353 - channel : list of nicks in channel
	// 366 - end of /NAMES list.
	// 375 - start of motd
	// 372 - motd
	// 353 - channel members
	// 366 - end of list of channel members
	// 403 - channel not found
	// 421 - unknown command
	// 433 - :Nickname is already in use.
	// 705 - reply to HELP command
	// 706 - end of reply to HELP command
	// MODE - read-write modes

	// Run I/O loop in it's own thread.
	_loop = new std::thread(&IRChatStream::looper, this);
}

// ==================================================================

ValuePtr IRChatStream::describe(AtomSpace* as, bool silent)
{
	throw RuntimeException(TRACE_INFO, "Not implemeneted");
	return Handle::UNDEFINED;
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

#if DEBUG
	printf(">>> IRC nick=%s ident=%s host=%s target=%s\n",
		ird->nick, ird->ident, ird->host, ird->target);
	printf(">>> IRC reply params=%s\n", params);
#endif
}

/// Do not attempt to join any channels, until MOTD has arrived.
/// XXX Implement me as a semaphore or mutex or something.
int IRChatStream::end_of_motd(const char* params, irc_reply_data* ird)
{
	return 0;
}

int IRChatStream::got_misc(const char* params, irc_reply_data* ird)
{
	fixup_reply(ird);

	// ident is the "true identity" of the nick.
	// host is the hostname that the nick is sending from,
	// target is who the message is for.
	printf("Misc reply: nick=%s nick-ident=%s nick-host=%s msg-target=%s\n",
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
	const char * start = params;
	if (':' == *start) start++;

	// ird->nick is who the message is from.
	// ird->target is who the message is to. It typically has one
	// of two values: the channel name, and so a public message, or
	// my nick, in which case its a private message to me.
	ValueSeq msg;
	msg.push_back(createStringValue(ird->nick));
	msg.push_back(createStringValue(ird->target));

	// Does the message start with the _nick? If so, special case it.
	// This is special handling to make message processing easier.
	if (0 == _nick.compare(0, _nick.size(), start, _nick.size()))
		msg.push_back(createStringValue(
			std::vector<std::string>({_nick, start + _nick.size()})));
	else
		msg.push_back(createStringValue(start));

	ValuePtr svp(createLinkValue(msg));
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
	// This will hang, until there's something to read.
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

#define CHKNARG(NUM,MSG) \
	if (cmdstrs.size() < NUM) \
		throw RuntimeException(TRACE_INFO, MSG);

/// Deal with IRC commands
/// Expecting structured values:
/// command-name + arguments
void IRChatStream::run_cmd(const std::vector<std::string>& cmdstrs)
{
	if (0 == cmdstrs.size()) return;

	const std::string& cmd = cmdstrs[0];
	if (0 == cmd.compare("PRIVMSG"))
	{
		CHKNARG(3, "Expecting at least three arguments");
		const char* msg_target = cmdstrs[1].c_str();

		if (cmdstrs.size() == 3)
		{
			_conn->privmsg(msg_target, cmdstrs[2].c_str());
			return;
		}

		// Concatenate the rest into a single line.
		std::string msg;
		for (size_t i=2; i< cmdstrs.size(); i++)
			msg += cmdstrs[i];
		_conn->privmsg(msg_target, msg.c_str());
		return;
	}

	if (0 == cmd.compare("JOIN"))
	{
		CHKNARG(2, "Expecting at least two arguments");
		// Channels must always start with hash mark.
		const char* channel = cmdstrs[1].c_str();
		// if ('#' != _channel[0]) _channel = "#" + _channel;
		_conn->join(channel);
		return;
	}

	if (0 == cmd.compare("PART"))
	{
		CHKNARG(2, "Expecting at least two arguments");
		const char* channel = cmdstrs[1].c_str();
		_conn->part(channel);
		return;
	}

	// Anything goes. Raw commands. Server will bitch if we do
	// something wrong.
	_conn->raw(cmd.c_str());
}

// ==============================================================
/// Deal with different kinds of stream formats.
void IRChatStream::prt_value(const ValuePtr& command_data)
{
	if (command_data->is_type(STRING_VALUE))
	{
		StringValuePtr svp(StringValueCast(command_data));
		run_cmd(svp->value());
		return;
	}

	if (command_data->is_type(LINK_VALUE))
	{
		// Syntactic sugar: unwrap a doubly-wrapped LinkValue
		// These arise "naturally" as a result of filtering,
		// and are a pain in the next to deal with. So we'll
		// just do it "for free" here.
		LinkValuePtr lvp(LinkValueCast(command_data));
		if (lvp->size() == 1 and lvp->value()[0]->is_type(LINK_VALUE))
			lvp = LinkValueCast(lvp->value()[0]);

		// Do it again. Sometimes get double-wrapped! Ugh! WTF.
		if (lvp->size() == 1 and lvp->value()[0]->is_type(LINK_VALUE))
			lvp = LinkValueCast(lvp->value()[0]);

		std::vector<std::string> cmd;
		for (const ValuePtr& vp : lvp->value())
		{
			if (vp->is_type(STRING_VALUE))
			{
				const StringValuePtr& svp(StringValueCast(vp));
				for (const std::string& str : svp->value())
					cmd.push_back(str);
			}
			else
			if (vp->is_node())
				cmd.push_back(HandleCast(vp)->get_name());

			// FalseLink is the result of a no-op. So do nothing.
			else if (FALSE_LINK == vp->get_type())
				return;

			else
				throw RuntimeException(TRACE_INFO,
					"Expecting node or string; got %s\n", vp->to_string().c_str());
		}
		run_cmd(cmd);
		return;
	}

	if (LIST_LINK == command_data->get_type())
	{
		std::vector<std::string> cmd;
		for (const Handle& ho : HandleCast(command_data)->getOutgoingSet())
		{
			if (not ho->is_node())
				throw RuntimeException(TRACE_INFO,
					"Expecting node; got %s\n", ho->to_string().c_str());
			cmd.push_back(ho->get_name());
		}
		run_cmd(cmd);
		return;
	}

	throw RuntimeException(TRACE_INFO,
		"Unsupported data %s\n", command_data->to_string().c_str());
}

// Write stuff to a file.
ValuePtr IRChatStream::write_out(AtomSpace* as, bool silent,
                                 const Handle& cref)
{
	if (nullptr == _conn)
		throw RuntimeException(TRACE_INFO,
			"IRC stream not open: URI \"%s\"\n", _uri.c_str());

	return do_write_out(as, silent, cref);
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
