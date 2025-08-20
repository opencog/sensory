/*
 * opencog/atoms/sensory/IRChatNode.h
 *
 * Copyright (C) 2024.2025 Linas Vepstas
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

#ifndef _OPENCOG_I_R_CHAT_NODE_H
#define _OPENCOG_I_R_CHAT_NODE_H

#include <thread>
#include <opencog/util/concurrent_queue.h>
#include <opencog/atoms/sensory/TextWriterNode.h>

class IRC;
struct irc_reply_data;

namespace opencog
{

/** \addtogroup grp_atomspace
 *  @{
 */

/**
 * IRChatNodes provide a stream of ItemNodes read from an IRC chat
 * channel. This is experimental.
 */
class IRChatNode
	: public TextWriterNode, protected concurrent_queue<ValuePtr>
{
private:
	IRC* _conn;
	std::thread* _loop;
	bool _cancel;
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
	IRChatNode(Type t, const std::string&&);

	std::string _uri;
	std::string _nick;
	std::string _host;
	int _port;

	virtual void write_one(const ValuePtr&);
	void run_cmd(const std::vector<std::string>&);

	virtual void open(const ValuePtr&);
	virtual void close(const ValuePtr&);
	// virtual void write(const ValuePtr&); inherited from TextWriterNode
	virtual bool connected(void) const;
	virtual ValuePtr read(void) const;

public:
	IRChatNode(const std::string&&);
	IRChatNode(Type, const std::string&&);
	virtual ~IRChatNode();

	static Handle factory(const Handle&);
};

NODE_PTR_DECL(IRChatNode)
#define createIRChatNode CREATE_DECL(IRChatNode)

/** @}*/
} // namespace opencog

extern "C" {
void opencog_sensory_irc_init(void);
};

#endif // _OPENCOG_I_R_CHAT_NODE_H
