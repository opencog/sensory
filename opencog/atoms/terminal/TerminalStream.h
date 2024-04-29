/*
 * opencog/atoms/sensory/TerminalStream.h
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

#ifndef _OPENCOG_TERMINAL_STREAM_H
#define _OPENCOG_TERMINAL_STREAM_H

#include <stdio.h>
#include <opencog/atoms/sensory/OutputStream.h>

namespace opencog
{

/** \addtogroup grp_atomspace
 *  @{
 */

/**
 * TerminalStreams provide a stream of ItemNodes read from a text file,
 * and, more generally, from unix socket sources. This is experimental.
 */
class TerminalStream
	: public OutputStream
{
protected:
	TerminalStream(Type t, const std::string&);
	void init(const std::string&);
	virtual void update() const;

	std::string _uri;
	mutable FILE* _fh;
	pid_t _xterm;
	virtual void do_write(const std::string&);

public:
	TerminalStream(const Handle&);
	TerminalStream(const std::string&);
	TerminalStream(const ValueSeq&);
	virtual ~TerminalStream();

	virtual ValuePtr describe(AtomSpace*, bool);
	virtual ValuePtr write_out(AtomSpace*, bool, const Handle&);
};

typedef std::shared_ptr<TerminalStream> TerminalStreamPtr;
static inline TerminalStreamPtr TerminalStreamCast(ValuePtr& a)
	{ return std::dynamic_pointer_cast<TerminalStream>(a); }

template<typename ... Type>
static inline std::shared_ptr<TerminalStream> createTerminalStream(Type&&... args) {
   return std::make_shared<TerminalStream>(std::forward<Type>(args)...);
}

/** @}*/
} // namespace opencog

extern "C" {
void opencog_sensory_terminal_init(void);
};

#endif // _OPENCOG_TERMINAL_STREAM_H
