/*
 * opencog/atoms/sensory-v0/SensoryNode.cc
 *
 * Copyright (c) 2008-2010 OpenCog Foundation
 * Copyright (c) 2009,2013,2020,2022 Linas Vepstas
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

#include <string>

#include <opencog/atomspace/AtomSpace.h>
#include <opencog/atoms/value/StringValue.h>
#include "SensoryNode.h"

using namespace opencog;

// ====================================================================

SensoryNode::SensoryNode(Type t, const std::string&& uri) :
	Node(t, std::move(uri))
{
	if (not nameserver().isA(t, SENSORY_NODE))
		throw RuntimeException(TRACE_INFO, "Bad inheritance!");
}

SensoryNode::~SensoryNode()
{
}

ValuePtr SensoryNode::getValue(const Handle& key) const
{
	if (not key->is_type(PREDICATE_NODE))
		return Atom::getValue(key);

	// Dispatch
	if (0 == key->get_name().compare("*-monitor-*"))
		return createStringValue(monitor());

	return Atom::getValue(key);
}

std::string SensoryNode::monitor(void) const
{
	return "This SensoryNode does not implement a monitor.";
}

// ====================================================================

void opencog_sensory_init(void)
{
   // Force shared lib ctors to run
};
