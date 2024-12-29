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
	// Nothing, just right now.
}

// ---------------------------------------------------------------

/// When evaluated, compare the streams, while papering over any
/// cracks that appear. The goal is to provide some easy-to-use
/// crack-pipe conversions. I was tired when I wrote this.
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
			comps.push_back(ho->execute(as, silent));
		else
			comps.push_back(ho);
	}

	return true;
}

DEFINE_LINK_FACTORY(StreamEqualLink, STREAM_EQUAL_LINK)

/* ===================== END OF FILE ===================== */
