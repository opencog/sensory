/*
 * opencog/atoms/sensory/TextStreamNode.cc
 *
 * Copyright (C) 2024, 2025 Linas Vepstas
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

#include <errno.h>
#include <string.h> // for strerror()

#include <opencog/util/exceptions.h>
#include <opencog/util/oc_assert.h>
#include <opencog/atoms/base/Link.h>
#include <opencog/atoms/base/Node.h>
#include <opencog/atoms/value/StringValue.h>
#include <opencog/atoms/value/VoidValue.h>

#include <opencog/sensory/types/atom_types.h>
#include "TextStream.h"
#include "TextStreamNode.h"

using namespace opencog;

TextStreamNode::TextStreamNode(Type t, const std::string&& url)
	: StreamNode(t, std::move(url))
{
	OC_ASSERT(nameserver().isA(_type, STREAM_NODE),
		"Bad TextStreamNode constructor!");
}

TextStreamNode::~TextStreamNode()
{
	printf ("TextStreamNode dtor\n");
}

// ==============================================================

// Decode the Item type.
void TextStreamNode::open(const ValuePtr& item_type)
{
	StreamNode::open(item_type);

	if (not nameserver().isA(_item_type, STRING_VALUE) and
	    not nameserver().isA(_item_type, NODE))
		throw RuntimeException(TRACE_INFO,
			"Expecting the type to be a StringValue or Node; got %s\n",
			item_type->to_string().c_str());
}

// ==============================================================

// Reader utility
ValuePtr TextStreamNode::string_to_type(std::string str) const
{
	if (0 == str.length()) return createVoidValue();

	// If a StringValue was asked for, get them that.
	if (nameserver().isA(_item_type, STRING_VALUE))
		return createStringValue(std::move(str));

	// Else it's some kind of Node.
	return createNode(_item_type, std::move(str));
}

ValuePtr TextStreamNode::read(void) const
{
	return string_to_type(do_read());
}

std::string TextStreamNode::do_read(void) const
{
	return std::string();
}

// ==============================================================

// Unpack strings.
void TextStreamNode::do_write(const ValuePtr& content)
{
	if (content->is_type(STRING_VALUE))
	{
		StringValuePtr svp(StringValueCast(content));
		const std::vector<std::string>& strs = svp->value();
		for (const std::string& str : strs)
			do_write(str);
		return;
	}
	if (content->is_type(NODE))
	{
		do_write(HandleCast(content)->get_name());
		return;
	}

	throw RuntimeException(TRACE_INFO,
		"Expecting strings, got %s\n", content->to_string().c_str());
}

// ==============================================================

// Override stream() to return TextStream instead of ReadStream
ValuePtr TextStreamNode::stream(void) const
{
	return createTextStream(get_handle());
}

// ==============================================================
