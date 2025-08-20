/*
 * opencog/atoms/sensory/TextWriterNode.h
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

#ifndef _OPENCOG_TEXT_WRITER_NODE_H
#define _OPENCOG_TEXT_WRITER_NODE_H

#include <opencog/atoms/sensory/SensoryNode.h>

namespace opencog
{

/** \addtogroup grp_atomspace
 *  @{
 */

/**
 * TextWriterNodeNode provides a virtual base class for objects that will
 * be writing (utf8 or ascii) text. It consists of some utility methods
 * that unpack, unwrap an assortment of Atoms, Values and Streams that
 * might occur that have text flowing on them.
 *
 * Basically, it is a conversion shim that allows a loose assortment of
 * almost anything to be streamed in, converting it into c++ strings
 * that are easy to handle to the actual writer.
 *
 * This API is experimental.
 */
class TextWriterNode
	: public SensoryNode
{
protected:
	TextWriterNode(Type t, const std::string&&);

	// Derived classes to implement this.
	virtual void do_write(const std::string&) = 0;

	// Derived classes might implement this.
	virtual void write_one(const ValuePtr&);

	// The "main" write routine, accepts anything.
	// Derived classes probably should NOT override this;
	// if they are, the are probably doing something wrong.
	virtual void write(const ValuePtr&);

public:
	virtual ~TextWriterNode();
};

NODE_PTR_DECL(TextWriterNode)

/** @}*/
} // namespace opencog

#endif // _OPENCOG_TEXT_WRITER_NODE_H
