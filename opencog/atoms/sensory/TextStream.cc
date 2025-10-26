/*
 * opencog/atoms/sensory/TextStream.cc
 *
 * Copyright (C) 2025 BrainyBlaze Dynamics, Inc.
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
#include "TextStream.h"

using namespace opencog;

TextStream::TextStream(const Handle& senso)
	: StringValue(TEXT_STREAM, std::vector<std::string>())
{
	if (not senso->is_type(SENSORY_NODE))
		throw RuntimeException(TRACE_INFO,
			"Expecting SensoryNode, got %s\n", senso->to_string().c_str());

	_snp = SensoryNodeCast(senso);
}

TextStream::~TextStream()
{
}

// ==============================================================

// This will read one item at a time from the object, and return
// that item as a string.
// So, still, a line-oriented, buffered interface. For now.
//
// This accesses the SensoryNode private methods directly; the
// alternative would be to call the public getValue() method, but
// that does nothing except add overhead.
void TextStream::update() const
{
	if (not _snp->connected())
	{
		_value.clear();
		return;
	}

	ValuePtr vp = _snp->read();

	// nullpointr and VoidValue denote explicit EOF
	if (nullptr == vp or VOID_VALUE == vp->get_type())
	{
		_value.clear();
		return;
	}

	// Convert the value to a string
	std::string str;
	if (vp->is_type(STRING_VALUE))
	{
		std::vector<std::string> next = StringValueCast(vp)->value();
		_value.swap(next);
		return;
	}

	if (vp->is_type(NODE))
	{
		_value.resize(1);
		_value[0] = HandleCast(vp)->get_name();
		return;
	}

	// Don't know what else to do, if not handled above.
	_value.clear();
}

// ==============================================================
// XXX TODO. Someday, we need a working to_short_string() that
// can be used by RocksStorage to ... store this thing. I guess.

std::string TextStream::to_string(const std::string& indent) const
{
   std::string rv = indent + "(" + nameserver().getTypeName(_type);
   rv += "\n" + _snp->to_short_string(indent + "   ") + ")\n";
   rv += indent + "; Current sample:\n";
   rv += StringValue::to_string(indent + "; ", STRING_VALUE);
   return rv;
}

// ==============================================================

// Adds factory when library is loaded.
DEFINE_VALUE_FACTORY(TEXT_STREAM, createTextStream, Handle)

// ====================================================================
