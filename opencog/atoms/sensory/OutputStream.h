/*
 * opencog/atoms/sensory/OutputStream.h
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

#ifndef _OPENCOG_OUTPUT_STREAM_H
#define _OPENCOG_OUTPUT_STREAM_H

#include <opencog/atoms/value/LinkStreamValue.h>

namespace opencog
{

/** \addtogroup grp_atomspace
 *  @{
 */

/**
 * OutputStream provides a virtual base class for streams that can write
 * things out.
 * This API is experimental.
 */
class OutputStream
	: public LinkStreamValue
{
protected:
	OutputStream(Type t);

	virtual void do_write(const std::string&);
	virtual void prt_value(const ValuePtr&);
	virtual ValuePtr do_write_out(AtomSpace*, bool, const Handle&);

public:
	virtual ~OutputStream();
	virtual ValuePtr write_out(AtomSpace*, bool, const Handle&) = 0;
	virtual ValuePtr describe(AtomSpace*, bool) = 0;

	// XXX Do we really need this?
	virtual bool operator==(const Value&) const;
};

typedef std::shared_ptr<OutputStream> OutputStreamPtr;
static inline OutputStreamPtr OutputStreamCast(ValuePtr& a)
	{ return std::dynamic_pointer_cast<OutputStream>(a); }


/** @}*/
} // namespace opencog

#endif // _OPENCOG_OUTPUT_STREAM_H
