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
	: UnorderedLink({ha, hb}, STREAM_EQUAL_LINK)
{
	init();
}

void StreamEqualLink::init(void)
{
	if (2 != _outgoing.size())
		throw SyntaxException(TRACE_INFO,
			"Expecting exactly two arguments!");

	if (not _outgoing[0]->is_executable())
		throw SyntaxException(TRACE_INFO,
			"Expecting the first argument to be executable!");
}

// ---------------------------------------------------------------

/// When executed, obtain the stream to write to, from the first
/// elt in outgoing set, and the data to write from the second.
ValuePtr StreamEqualLink::execute(AtomSpace* as, bool silent)
{
	ValuePtr pap = _outgoing[0]->execute(as, silent);
	if (nullptr == pap)
		throw RuntimeException(TRACE_INFO,
			"Expecting an OutputStream, but have nothing from %s",
			_outgoing[0]->to_string().c_str());

	OutputStreamPtr ost(OutputStreamCast(pap));
	if (nullptr == ost)
		throw RuntimeException(TRACE_INFO,
			"Expecting an OutputStream, got %s", pap->to_string().c_str());

	// XXX FIXME. Some sensory nodes, e.g. FileSysStream, will throw
	// exceptions when e.g. file permissions disallow some action.
	// Long term architecture should probably be to pass these up
	// to higher layers for processing. For now, we don't have that
	// in place, so instead, just suppress the exception. This
	// should change, someday.
	try
	{
		return ost->write_out(as, silent, _outgoing[1]);
	}
	catch (const StandardException& ex)
	{
		return createVoidValue();
	}
}

DEFINE_LINK_FACTORY(StreamEqualLink, STREAM_EQUAL_LINK)

/* ===================== END OF FILE ===================== */
