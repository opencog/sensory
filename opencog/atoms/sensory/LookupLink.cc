/*
 * LookupLink.cc
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

#include <opencog/atoms/core/TypeNode.h>
#include <opencog/atoms/value/ValueFactory.h>
#include "LookupLink.h"
#include "OutputStream.h"

using namespace opencog;

LookupLink::LookupLink(const HandleSeq&& oset, Type t)
	: Link(std::move(oset), t)
{
	if (not nameserver().isA(t, LOOKUP_LINK))
	{
		const std::string& tname = nameserver().getTypeName(t);
		throw InvalidParamException(TRACE_INFO,
			"Expecting an LookupLink, got %s", tname.c_str());
	}
	init();
}

LookupLink::LookupLink(const Handle& h)
	: Link({h}, LOOKUP_LINK)
{
	init();
}

void LookupLink::init(void)
{
	if (1 != _outgoing.size())
		throw SyntaxException(TRACE_INFO,
			"Expecting exactly onet argument!");

	if (TYPE_NODE != _outgoing[0]->get_type())
		throw SyntaxException(TRACE_INFO,
			"Expecting the argument to be a type!");

	_kind = TypeNodeCast(_outgoing[0])->get_kind();
}

// ---------------------------------------------------------------

/// When executed, get the stream description for the given type.
ValuePtr LookupLink::execute(AtomSpace* as, bool silent)
{
	ValuePtr svp = valueserver().create(_kind);

	OutputStreamPtr ost(OutputStreamCast(svp));
	if (nullptr == ost)
		throw RuntimeException(TRACE_INFO,
			"No support for %s", _outgoing[0]->to_string().c_str());
	return ost;
}

DEFINE_LINK_FACTORY(LookupLink, LOOKUP_LINK)

/* ===================== END OF FILE ===================== */
