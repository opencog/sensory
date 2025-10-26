/*
 * opencog/atoms/sensory/TextStreamNode.h
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

#ifndef _OPENCOG_TEXT_STREAM_NODE_H
#define _OPENCOG_TEXT_STREAM_NODE_H

#include <opencog/atoms/sensory/StreamNode.h>

namespace opencog
{

/** \addtogroup grp_atomspace
 *  @{
 */

/**
 * TextStreamNode provides a virtual base class for objects that will
 * be writing (utf8 or ascii) text. It consists of some utility methods
 * that unpack, unwrap an assortment of Atoms, Values and TextStreams that
 * might occur that have text flowing on them.
 *
 * Basically, it is a conversion shim that allows a loose assortment of
 * almost anything to be streamed in, converting it into c++ strings
 * that are easy to handle to the actual writer.
 *
 * This API is experimental.
 */
class TextStreamNode
	: public StreamNode
{
protected:
	ValuePtr string_to_type(std::string) const;

	TextStreamNode(Type t, const std::string&&);
	virtual void open(const ValuePtr&);

	virtual ValuePtr read(void) const;
	virtual std::string do_read(void) const;

	virtual void do_write(const ValuePtr&);

	// Derived classes need to implement a handler.
	virtual void do_write(const std::string&) = 0;

	// Override to return TextStream instead of ReadStream
	virtual ValuePtr stream(void) const;

public:
	virtual ~TextStreamNode();
};

NODE_PTR_DECL(TextStreamNode)

/** @}*/
} // namespace opencog

#endif // _OPENCOG_TEXT_STREAM_NODE_H
