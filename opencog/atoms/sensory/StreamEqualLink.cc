/*
 * opencog/atoms/sensory/StreamEqualLink.cc
 *
 * Copyright (C) 2022, 2024 Linas Vepstas
 *
 * Author: Linas Vepstas <linasvepstas@gmail.com>  January 2009
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Affero General Public License v3 as
 * published by the Free Software Foundation and including the
 * exceptions at http://opencog.org/wiki/Licenses
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Affero General Public
 * License along with this program; if not, write to:
 * Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <opencog/atoms/core/TypeNode.h>
#include <opencog/atoms/value/LinkStreamValue.h>
#include <opencog/atoms/value/StringValue.h>
#include <opencog/util/exceptions.h>

#include "StreamEqualLink.h"

using namespace opencog;

StreamEqualLink::StreamEqualLink(const HandleSeq&& oset, Type t)
	: UnorderedLink(std::move(oset), t)
{
	if (not nameserver().isA(t, STREAM_EQUAL_LINK))
	{
		const std::string& tname = nameserver().getTypeName(t);
		throw InvalidParamException(TRACE_INFO,
			"Expecting an StreamEqualLink, got %s", tname.c_str());
	}
	init();
}

StreamEqualLink::StreamEqualLink(const Handle& ha, const Handle& hb)
	: UnorderedLink(HandleSeq({ha, hb}), STREAM_EQUAL_LINK)
{
	init();
}

void StreamEqualLink::init(void)
{
	if (_outgoing.size() > 2)
		throw RuntimeException(TRACE_INFO,
			"Compare of more than two stream items not implemented.");
}

// ---------------------------------------------------------------

/// When evaluated, compare the first item in a stream to an Atom,
/// of the first item in two different streams.  StringValue to
/// Node name compares succeed, if the actual strings match; the
/// Node type is discarded.
///
/// Only the first item in the stream is compared. This works great
/// for sampled streams (e.g. "is the door open, right now?") but
/// might be inappropriate for buffered streams. Depends on the
/// stream implementation. We're fast and loose here.
bool StreamEqualLink::bevaluate(AtomSpace* as, bool silent)
{
	// Self-equality is trivially true. Or we could throw an
	// exception here.
	if (_outgoing.size() < 2)
		return true;

	ValueSeq comps;
	for (const Handle& ho: _outgoing)
	{
		if (ho->is_executable())
		{
			ValuePtr vp = ho->execute(as, silent);

			// If its a stream, explicitly get only the first item
			// in the stream. (This may cause unintended data loss
			// in buffered streams).
			if (vp->is_type(LINK_STREAM_VALUE))
				vp = LinkStreamValueCast(vp)->value()[0];
			comps.push_back(vp);
		}
		else
			comps.push_back(ho);
	}

	return compare(comps[0], comps[1]);
}

bool StreamEqualLink::compare(ValuePtr vpa, ValuePtr vpb)
{
	// Sort. Nodes come first.
	if (vpb->is_node()) vpb.swap(vpa);

	// Do string compares, ignoring node type.
	if (vpa->is_node())
	{
		// Anonymous type compare
		if (vpa->is_type(TYPE_NODE))
		{
			Type wtype = TypeNodeCast(HandleCast(vpa))->get_kind();
			return vpb->is_type(wtype);
		}

		// Node name compare; ignore node type
		if (vpb->is_node())
		{
			const std::string& na(HandleCast(vpa)->get_name());
			const std::string& nb(HandleCast(vpb)->get_name());
			return 0 == na.compare(nb);
		}
		if (not vpb->is_type(STRING_VALUE))
			return false;

		// String compare
		StringValuePtr svp(StringValueCast(vpb));
		if (1 < svp->size())
			return false;

		const std::string& na(HandleCast(vpa)->get_name());
		const std::string& nb(svp->value()[0]);
		return 0 == na.compare(nb);
	}

	// Do string vector compares.
	if (vpa->is_type(STRING_VALUE))
	{
		if (not vpb->is_type(STRING_VALUE))
			return false;
		if (vpa->size() != vpb->size())
			return false;

		const std::vector<std::string>& sva(StringValueCast(vpa)->value());
		const std::vector<std::string>& svb(StringValueCast(vpb)->value());
		for (size_t i=0; i<sva.size(); i++)
		{
			if (sva[i].compare(svb[i]))
				return false;
		}
		return true;
	}

	// Sort. Links come first.
	if (vpb->is_link()) vpb.swap(vpa);

	// Do link compares. Ignore link type, if the other side
	// is a LinkStream. Otherwise do strict link compares.
	if (vpa->is_link())
	{
		if (vpb->is_link())
			return *vpa == *vpb;
		if (not vpb->is_type(LINK_VALUE))
			return false;
		if (vpa->size() != vpb->size())
			return false;

		const HandleSeq& oset(HandleCast(vpa)->getOutgoingSet());
		const ValueSeq& vseq(LinkValueCast(vpb)->value());
		for (size_t i=0; i<oset.size(); i++)
		{
			if (not compare(oset[i], vseq[i]))
				return false;
		}
		return true;
	}

	// I don't know what's happening, if vpa isn't a link stream.
	if (not vpa->is_type(LINK_VALUE))
		return false;

	if (vpa->size() != vpb->size())
		return false;

	const ValueSeq& vqa(LinkValueCast(vpa)->value());
	const ValueSeq& vqb(LinkValueCast(vpb)->value());
	for (size_t i=0; i<vqa.size(); i++)
	{
		if (not compare(vqa[i], vqb[i]))
			return false;
	}
	return true;
}

DEFINE_LINK_FACTORY(StreamEqualLink, STREAM_EQUAL_LINK)

/* ===================== END OF FILE ===================== */
