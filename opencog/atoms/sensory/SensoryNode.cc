/*
 * opencog/atoms/sensory/SensoryNode.cc
 *
 * Copyright (c) 2008-2010 OpenCog Foundation
 * Copyright (c) 2009,2013,2020,2022,2025 Linas Vepstas
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
#include <opencog/atoms/value/BoolValue.h>
#include <opencog/atoms/value/StringValue.h>
#include "DispatchHash.h"
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

std::string SensoryNode::monitor(void) const
{
	return "This SensoryNode does not implement a monitor.";
}

void SensoryNode::barrier(AtomSpace* as)
{
	if (nullptr == as) as = getAtomSpace();
	as->barrier();
}

void SensoryNode::follow(const ValuePtr& value)
{
	// Default: do nothing. Derived classes can override.
}

void SensoryNode::setValue(const Handle& key, const ValuePtr& value)
{
	// The value must be store only if it is not one of the values
	// that causes an action to be taken. Action messages must not be
	// recorded, as otherwise, restore from disk/net will cause the
	// action to be triggered!
	if (PREDICATE_NODE != key->get_type())
	{
		Atom::setValue(key, value);
		return;
	}

	// Create a fast dispatch table using case-statement branching.
	static constexpr uint32_t p_open =
		dispatch_hash("*-open-*");
	static constexpr uint32_t p_close =
		dispatch_hash("*-close-*");
	static constexpr uint32_t p_write =
		dispatch_hash("*-write-*");
	static constexpr uint32_t p_barrier = dispatch_hash("*-barrier-*");
	static constexpr uint32_t p_follow = dispatch_hash("*-follow-*");

// There's almost no chance at all that any user will use some key
// that is a PredicateNode that has a string name that collides with
// one of the above. That's because there's really no reason to set
// static values on a SensoryNode. That I can think of. Still there's
// some chance of a hash collision. In this case, define
// COLLISION_PROOF, and recompile. Sorry in advance for the awful
// debug session you had that caused you to discover this comment!
//
// #define COLLISION_PROOF
#ifdef COLLISION_PROOF
	#define COLL(STR) if (0 != pred.compare(STR)) break;
#else
	#define COLL(STR)
#endif

	const std::string& pred = key->get_name();
	switch (dispatch_hash(pred.c_str()))
	{
		case p_open:
			COLL("*-open-*");
			open(value);
			return;
		case p_close:
			COLL("*-close-*");
			close(value);
			return;
		case p_write:
			COLL("*-write-*");
			write(value);
			return;
		case p_barrier:
			COLL("*-barrier-*");
			barrier(AtomSpaceCast(value).get());
			return;
		case p_follow:
			COLL("*-follow-*");
			follow(value);
			return;
		default:
			break;
	}

	// Some other predicate. Store it.
	Atom::setValue(key, value);
}

ValuePtr SensoryNode::getValue(const Handle& key) const
{
	if (PREDICATE_NODE != key->get_type())
		return Atom::getValue(key);

	// Create a fast dispatch table using case-statement branching.
	static constexpr uint32_t p_connected_p =
		dispatch_hash("*-connected?-*");
	static constexpr uint32_t p_read =
		dispatch_hash("*-read-*");
	static constexpr uint32_t p_stream =
		dispatch_hash("*-stream-*");
	static constexpr uint32_t p_monitor =
		dispatch_hash("*-monitor-*");

	const std::string& pred(key->get_name());
	switch (dispatch_hash(pred.c_str()))
	{
		case p_read:
			COLL("*-read-*");
			return read();
		case p_stream:
			COLL("*-stream-*");
			return stream();
		case p_connected_p:
			COLL("*-connected?-*");
			return createBoolValue(connected());
		case p_monitor:
			COLL("*-monitor-*");
			return createStringValue(monitor());
		default:
			break;
	}
	return Atom::getValue(key);
}

// ====================================================================

void opencog_sensory_init(void)
{
	// Force shared lib ctors to run
};
