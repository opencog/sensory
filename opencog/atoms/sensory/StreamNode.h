/*
 * opencog/atoms/sensory/StreamNode.h
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

#ifndef _OPENCOG_STREAM_NODE_H
#define _OPENCOG_STREAM_NODE_H

#include <opencog/atoms/sensory/SensoryNode.h>

namespace opencog
{

/** \addtogroup grp_atomspace
 *  @{
 */

/**
 * StreamNode provides a virtual base class for managing line-oriented
 * objects and presenting a streaming interface for them. In particular,
 * it enables the construction of streams that will run indefinitely
 * i.e. looping for as long as an input stream remains open.
 *
 * This API is experimental.
 * See DesignNotes-J.md for detailed design considerations.
 */
class StreamNode
	: public SensoryNode
{
protected:
	StreamNode(Type t, const std::string&&);

	// Derived classes to implement this.
	virtual void do_write(const ValuePtr&) = 0;

	// Write out just one item. Provides a reasonable default;
   // Derived classes might wish to provide an alternative
	// implementation,
	virtual void write_one(const ValuePtr&);

	// The "main" write routine, accepts anything.
	// Derived classes probably should NOT override this;
	// if they are, the are probably doing something wrong.
	virtual void write(const ValuePtr&);

public:
	virtual ~StreamNode();
};

NODE_PTR_DECL(StreamNode)

/** @}*/
} // namespace opencog

#endif // _OPENCOG_STREAM_NODE_H
