/*
 * opencog/atoms/sensory/OutputStream.cc
 *
 * Copyright (C) 2020 Linas Vepstas
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
#include <opencog/atoms/base/Node.h>
#include <opencog/atoms/value/StringValue.h>

#include <opencog/atoms/sensory-types/sensory_types.h>
#include "OutputStream.h"

using namespace opencog;

OutputStream::OutputStream(Type t)
	: LinkStreamValue(t)
{
	OC_ASSERT(nameserver().isA(_type, OUTPUT_STREAM),
		"Bad OutputStream constructor!");
}

OutputStream::~OutputStream()
{
	printf ("OutputStream dtor\n");
}

// ==============================================================

// Provide an unreasonable default implementation
void OutputStream::do_write(const std::string& str)
{
	throw RuntimeException(TRACE_INFO,
		"Not implemented! What are you doing?");
}

// Provide a reasonable default implementation
void OutputStream::prt_value(const ValuePtr& content)
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
	if (content->is_type(LINK_VALUE))
	{
		LinkValuePtr lvp(LinkValueCast(content));
		const ValueSeq& vals = lvp->value();
		for (const ValuePtr& v : vals)
			prt_value(v);
		return;
	}

	// Backwards-compat: AllowListLink and SetLink (only!?)
	// Why restrict? I dunno. Seems like the right thing to do.
	Type tc = content->get_type();
	if (LIST_LINK == tc or SET_LINK == tc)
	{
		const HandleSeq& oset = HandleCast(content)->getOutgoingSet();
		for (const Handle& h : oset)
			prt_value(h);
		return;
	}

	throw RuntimeException(TRACE_INFO,
		"Expecting strings, got %s\n", content->to_string().c_str());
}

// Provide a reasonable default implementation.
ValuePtr OutputStream::do_write_out(AtomSpace* as, bool silent,
                                    const Handle& cref)
{
	ValuePtr content = cref;
	if (cref->is_executable())
	{
		content = cref->execute(as, silent);
		if (nullptr == content)
			throw RuntimeException(TRACE_INFO,
				"Expecting something to write from %s\n",
				cref->to_string().c_str());
	}

	// If it is not a stream, then just print and return.
	if (not content->is_type(LINK_STREAM_VALUE))
	{
		prt_value(content);
		return content;
	}

	// If it is a stream, enter infinite loop, until it is exhausted.
	LinkValuePtr lvp(LinkValueCast(content));
	while (true)
	{
		const ValueSeq& vals = lvp->value();

		// If the stream is returning an empty list, assume we
		// are done. Exit the loop.
		if (0 == vals.size()) break;

		// A different case arises if the stream keeps returning
		// empty LinkValues. This is kind of pathological, and
		// arguably, its a bug upstream somewhere, but for now,
		// we catch this and handle it.
		size_t nprinted = 0;
		for (const ValuePtr& v : vals)
		{
			if (v->is_type(LINK_VALUE) and 0 == v->size()) continue;
			prt_value(v);
			nprinted ++;
		}
		if (0 == nprinted) break;
	}
	return content;
}

// ==============================================================
