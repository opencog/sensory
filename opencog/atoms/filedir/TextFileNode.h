/*
 * opencog/atoms/sensory/TextFileNode.h
 *
 * Copyright (C) 2024,2025 Linas Vepstas
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

#ifndef _OPENCOG_TEXT_FILE_NODE_H
#define _OPENCOG_TEXT_FILE_NODE_H

#include <stdio.h>
#include <mutex>
#include <opencog/atoms/sensory/TextStreamNode.h>
#include "FileWatcher.h"

namespace opencog
{

/** \addtogroup grp_atomspace
 *  @{
 */

/**
 * TextFileNodes provide a stream of StringValues read from a text file,
 * and, more generally, from unix socket sources.
 *
 * Provides ability to watch (tail) a file. This forces the design to
 * be thread safe, because the only way to break out of a watch (a
 * blocking read) in one thread is to call close() from a different
 * thread.
 *
 * This is experimental.
 * Unsolved issues:
 * -- Fails to trim newline at end of line.
 * -- Fails to handle lines longer than 4096
 * -- Fails to trim CRLF, if it's a DOS file.
 */
class TextFileNode
	: public TextStreamNode
{
protected:
	mutable std::mutex _mtx;  // Protects _fh and coordinates close/read
	mutable FILE* _fh;
	mutable bool _tail_mode;
	mutable FileWatcher _watcher;

	virtual void do_write(const std::string&);

	virtual void open(const ValuePtr&);
	virtual void close(const ValuePtr&);
	// virtual void write(const ValuePtr&); inherited from StreamNode
	virtual bool connected(void) const;
	virtual void barrier(AtomSpace* = nullptr);
	virtual void follow(const ValuePtr&);
	virtual std::string do_read(void) const;

public:
	TextFileNode(const std::string&&);
	TextFileNode(Type t, const std::string&&);
	virtual ~TextFileNode();

	static Handle factory(const Handle&);
};

NODE_PTR_DECL(TextFileNode)
#define createTextFileNode CREATE_DECL(TextFileNode)

/** @}*/
} // namespace opencog

extern "C" {
void opencog_sensory_filedir_init(void);
};

#endif // _OPENCOG_TEXT_FILE_NODE_H
