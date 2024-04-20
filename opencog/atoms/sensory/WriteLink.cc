/*
 * WriteLink.cc
 *
 * Copyright (C) 2022 Linas Vepstas
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

#include "WriteLink.h"
#include "OutputStream.h"

using namespace opencog;

WriteLink::WriteLink(const HandleSeq&& oset, Type t)
	: Link(std::move(oset), t)
{
	if (not nameserver().isA(t, WRITE_LINK))
	{
		const std::string& tname = nameserver().getTypeName(t);
		throw InvalidParamException(TRACE_INFO,
			"Expecting an WriteLink, got %s", tname.c_str());
	}
	init();
}

WriteLink::WriteLink(const Handle& h)
	: Link({h}, WRITE_LINK)
{
	init();
}

void WriteLink::init(void)
{
	if (2 != _outgoing.size())
		throw SyntaxException(TRACE_INFO,
			"Expecting exactly two arguments!");

	if (not _outgoing[0]->is_executable())
		throw SyntaxException(TRACE_INFO,
			"Expecting the first argument to be executable!");
}

// ---------------------------------------------------------------

/// When executed, obtain the steam to write to from the first
/// elt in outgoing set, and the data to write from the second.
ValuePtr WriteLink::execute(AtomSpace* as, bool silent)
{
	ValuePtr pap = _outgoing[0]->execute(as, silent);
	OutputStreamPtr ost(OutputStreamCast(pap));

	if (nullptr == ost)
		throw RuntimeException(TRACE_INFO,
			"Expecting an OutputStream, got %s", pap->to_string().c_str());

	return ost->write_out(as, silent, _outgoing[1]);
}

DEFINE_LINK_FACTORY(WriteLink, WRITE_LINK)

/* ===================== END OF FILE ===================== */
