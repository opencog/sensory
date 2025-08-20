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
#include <opencog/atoms/sensory/TextWriterNode.h>

namespace opencog
{

/** \addtogroup grp_atomspace
 *  @{
 */

/**
 * TextFileNodes provide a stream of ItemNodes read from a text file,
 * and, more generally, from unix socket sources. This is experimental.
 */
class TextFileNode
	: public TextWriterNode
{
protected:
	FILE* _fh;
	virtual void do_write(const std::string&);

	virtual void open(const ValuePtr&);
	virtual void close(const ValuePtr&);
	// virtual void write(const ValuePtr&); inherited from TextWriterNode
	virtual bool connected(void) const;
	virtual ValuePtr read(void) const;

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
