/*
 * opencog/atoms/sensory/StreamNode.cc
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
#include <opencog/atoms/core/TypeNode.h>
#include <opencog/atoms/value/LinkValue.h>
#include <opencog/atoms/value/ContainerValue.h>
#include <opencog/atoms/value/StringValue.h>

#include <opencog/sensory/types/atom_types.h>
#include "ReadStream.h"
#include "StreamNode.h"

using namespace opencog;

StreamNode::StreamNode(Type t, const std::string&& url)
	: SensoryNode(t, std::move(url))
{
	OC_ASSERT(nameserver().isA(_type, SENSORY_NODE),
		"Bad StreamNode constructor!");
}

StreamNode::~StreamNode()
{
	printf ("StreamNode dtor\n");
}

// ==============================================================

// Decode the Item type.
void StreamNode::open(const ValuePtr& item_type)
{
	// In the future, we could, in principle, accept wacky signatures
	// here, or complicated type constructors of some kind of another,
	// as long as they are able to construct something holding text
	// strings. But for now, keep it simple. Simple TypeNode, that's all.
	if (not item_type->is_type(TYPE_NODE))
		throw RuntimeException(TRACE_INFO,
			"Expecting a TypeNode; got %s\n", item_type->to_string().c_str());

	TypeNodePtr tp = TypeNodeCast(item_type);
	_item_type = tp->get_kind();
}

// ==============================================================

// Wrap ourselves in a streamer.
ValuePtr StreamNode::stream(void) const
{
	return createReadStream(get_handle());
}

// ==============================================================

// Provide a reasonable default implementation
void StreamNode::write_one(const ValuePtr& content)
{
	if (content->is_type(LINK_VALUE))
	{
		LinkValuePtr lvp(LinkValueCast(content));
		const ValueSeq& vals = lvp->value();
		for (const ValuePtr& v : vals)
			write_one(v);
		return;
	}

	// Backwards-compat: Allow ListLink and SetLink (only!?)
	// Why restrict? I dunno. Seems like the right thing to do.
	Type tc = content->get_type();
	if (LIST_LINK == tc or SET_LINK == tc)
	{
		const HandleSeq& oset = HandleCast(content)->getOutgoingSet();
		for (const Handle& h : oset)
			write_one(h);
		return;
	}

	// Hack: VoidValue typically means some reader reached end-of-file,
	// and the writer mindlessly copied from the reader, to us. Now,
	// the pipeline before the writer should have checked for end-of-file
	// and should never have sent us this bogus value. But they didn't,
	// and we received it. Throw a silent exception. This will avoid
	// spew, but it will break out of infinite loops.
	if (content->is_type(VOID_VALUE))
		throw SilentException();

	// Everything else falls through.
	do_write(content);
}

// Provide a reasonable default implementation.
void StreamNode::write(const ValuePtr& cref)
{
	ValuePtr content = cref;
	if (cref->is_atom() and HandleCast(cref)->is_executable())
	{
		content = HandleCast(cref)->execute();
		if (nullptr == content)
			throw RuntimeException(TRACE_INFO,
				"Expecting something to write from %s\n",
				cref->to_string().c_str());
	}

	// If it is not a stream, then just print and return.
	if (not content->is_type(LINK_VALUE))
	{
		write_one(content);
		return;
	}

	// If it is a container, enter infinite loop, until the container
	// is closed.
	if (content->is_type(CONTAINER_VALUE))
	{
		ContainerValuePtr cvp(ContainerValueCast(content));

		// If the container is open, that means some other thread
		// is stuffing values into it. Pull them out, one by one,
		// as they appear.
		while (not cvp->is_closed())
		{
			ValuePtr v(cvp->remove());
			if (v->is_type(LINK_VALUE) and 0 == v->size()) continue;
			write_one(v);
		}

		// We arrive here if the container is closed.
		// In this case, drain it, and we are done.
		const ValueSeq& vals = cvp->value();
		for (const ValuePtr& v : vals)
		{
			if (v->is_type(LINK_VALUE) and 0 == v->size()) continue;
			write_one(v);
		}
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
			write_one(v);
			nprinted ++;
		}
		if (0 == nprinted) break;
	}
}

// ==============================================================
