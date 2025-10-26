/*
 * opencog/atoms/sensory/ReadStream.cc
 *
 * Copyright (C) 2025 Linas Vepstas
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

#include <opencog/util/exceptions.h>
#include <opencog/util/oc_assert.h>
#include <opencog/atoms/base/Node.h>
#include <opencog/atoms/value/StringValue.h>
#include <opencog/atoms/value/ValueFactory.h>

#include <opencog/sensory/types/atom_types.h>
#include "ReadStream.h"

using namespace opencog;

ReadStream::ReadStream(const Handle& senso)
	: LinkValue(READ_STREAM)
{
	if (not senso->is_type(SENSORY_NODE))
		throw RuntimeException(TRACE_INFO,
			"Expecting SensoryNode, got %s\n", senso->to_string().c_str());

	_snp = SensoryNodeCast(senso);
}

ReadStream::~ReadStream()
{
}

// ==============================================================

// This will read one item at a time from the object, and return
// that item.
// So, still, a line-oriented, buffered interface. For now.
//
// This accesses the SensoryNode private methods directly; the
// alternative would be to call the public getValue() method, but
// that does nothing except add overhead.
void ReadStream::update() const
{
	if (not _snp->connected()) return;

	// _value.emplace_back(_snp->read());
	_value.resize(1);
	_value[0] = _snp->read();
}

// ==============================================================
// XXX TODO. Someday, we need a working to_short_string() that
// can be used by RocksStorage to ... store this thing. I guess.

std::string ReadStream::to_string(const std::string& indent) const
{
   std::string rv = indent + "(" + nameserver().getTypeName(_type);
   rv += "\n" + _snp->to_short_string(indent + "   ") + ")\n";
   rv += indent + "; Currently:\n";
   rv += LinkValue::to_string(indent + "; ", LINK_VALUE);
   return rv;
}

// ==============================================================

// Adds factory when library is loaded.
DEFINE_VALUE_FACTORY(READ_STREAM, createReadStream, Handle)

// ====================================================================
