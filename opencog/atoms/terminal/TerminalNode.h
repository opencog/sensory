/*
 * opencog/atoms/sensory/TerminalNode.h
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

#ifndef _OPENCOG_TERMINAL_NODE_H
#define _OPENCOG_TERMINAL_NODE_H

#include <mutex>
#include <stdio.h>
#include <opencog/atoms/sensory/TextStreamNode.h>

namespace opencog
{

/** \addtogroup grp_atomspace
 *  @{
 */

/**
 * TerminalNodes provide a text I/O via an instance of an xterm.
 * Due to the general weirdness of attaching terminals to running
 * processes, the implementation is kind of whack, because the
 * cleaner alternative is to build a telnet sserver, and I don't
 * want to do that. Not today.
 */
class TerminalNode
	: public TextStreamNode
{
protected:
	mutable std::mutex _mtx;
	void halt(void) const;

	mutable FILE* _fh;
	mutable pid_t _xterm_pid;
	virtual void do_write(const std::string&);

	virtual void open(const ValuePtr&);
	virtual void close(const ValuePtr&);
	// virtual void write(const ValuePtr&); inherited from TextWriterNode
	virtual bool connected(void) const;
	virtual std::string do_read(void) const;

public:
	TerminalNode(const std::string&&);
	TerminalNode(Type t, const std::string&&);
	virtual ~TerminalNode();

	static Handle factory(const Handle&);
};

NODE_PTR_DECL(TerminalNode)
#define createTerminalNode CREATE_DECL(TerminalNode)

/** @}*/
} // namespace opencog

extern "C" {
void opencog_sensory_terminal_init(void);
};

#endif // _OPENCOG_TERMINAL_NODE_H
