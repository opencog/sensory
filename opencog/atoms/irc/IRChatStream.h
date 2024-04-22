/*
 * opencog/atoms/sensory/IRChatStream.h
 *
 * Copyright (C) 2024 Linas Vepstas
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

#ifndef _OPENCOG_I_R_CHAT_STREAM_H
#define _OPENCOG_I_R_CHAT_STREAM_H

#include <thread>
#include <opencog/util/concurrent_queue.h>
#include <opencog/atoms/sensory/OutputStream.h>

class IRC;
struct irc_reply_data;

namespace opencog
{

/** \addtogroup grp_atomspace
 *  @{
 */

/**
 * IRChatStreams provide a stream of ItemNodes read from an IRC chat
 * channel. This is experimental.
 */
class IRChatStream
	: public OutputStream, protected concurrent_queue<ValuePtr>
{
private:
	IRC* _conn;
	std::thread* _loop;
	void looper(void);

	static int xend_of_motd(const char*, irc_reply_data*, void*);
	static int xgot_privmsg(const char*, irc_reply_data*, void*);
	static int xgot_kick(const char*, irc_reply_data*, void*);
	static int xgot_misc(const char*, irc_reply_data*, void*);

	int end_of_motd(const char*, irc_reply_data*);
	int got_privmsg(const char*, irc_reply_data*);
	int got_kick(const char*, irc_reply_data*);
	int got_misc(const char*, irc_reply_data*);

protected:
	IRChatStream(Type t, const std::string&);
	void init(const std::string&);
	virtual void update() const;

	std::string _uri;
	std::string _host;
	int _port;
	std::string _channel;
	std::string _nick;

	virtual void do_write(const std::string&);
	virtual void prt_value(const ValuePtr&);

public:
	IRChatStream(const Handle&);
	IRChatStream(const std::string&);
	virtual ~IRChatStream();

	virtual ValuePtr write_out(AtomSpace*, bool, const Handle&);
	virtual bool operator==(const Value&) const;
};

typedef std::shared_ptr<IRChatStream> IRChatStreamPtr;
static inline IRChatStreamPtr IRChatStreamCast(ValuePtr& a)
	{ return std::dynamic_pointer_cast<IRChatStream>(a); }

template<typename ... Type>
static inline std::shared_ptr<IRChatStream> createIRChatStream(Type&&... args) {
   return std::make_shared<IRChatStream>(std::forward<Type>(args)...);
}

/** @}*/
} // namespace opencog

extern "C" {
void opencog_sensory_irc_init(void);
};

#endif // _OPENCOG_I_R_CHAT_STREAM_H
