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
#include <opencog/atoms/value/LinkValue.h>
#include <opencog/atoms/value/StringValue.h>

#include <opencog/sensory/types/atom_types.h>
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
