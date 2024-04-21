/*
 * OpenLink.cc
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

#include "OpenLink.h"
#include "OutputStream.h"

using namespace opencog;

OpenLink::OpenLink(const HandleSeq&& oset, Type t)
	: Link(std::move(oset), t)
{
	if (not nameserver().isA(t, OPEN_LINK))
	{
		const std::string& tname = nameserver().getTypeName(t);
		throw InvalidParamException(TRACE_INFO,
			"Expecting an OpenLink, got %s", tname.c_str());
	}
	init();
}

OpenLink::OpenLink(const Handle& h)
	: Link({h}, OPEN_LINK)
{
	init();
}

void OpenLink::init(void)
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
ValuePtr OpenLink::execute(AtomSpace* as, bool silent)
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

	return ost->write_out(as, silent, _outgoing[1]);
}

DEFINE_LINK_FACTORY(OpenLink, OPEN_LINK)

/* ===================== END OF FILE ===================== */
