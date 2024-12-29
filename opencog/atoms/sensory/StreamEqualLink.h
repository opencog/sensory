/*
 * opencog/atoms/sensory/StreamEqualLink.h
 *
 * Copyright (C) 2018, 2024 Linas Vepstas
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

#ifndef _OPENCOG_STREAM_EQUAL_LINK_H
#define _OPENCOG_STREAM_EQUAL_LINK_H

#include <opencog/atoms/core/UnorderedLink.h>
#include <opencog/atoms/truthvalue/TruthValue.h>
#include <opencog/atoms/sensory-types/sensory_types.h>

namespace opencog
{
/** \addtogroup grp_atomspace
 *  @{
 */

/// The StreamEqualLink performs unification-equality.
///
class StreamEqualLink : public UnorderedLink
{
private:
	void init(void);

protected:
	bool compare(ValuePtr, ValuePtr);

public:
	StreamEqualLink(const HandleSeq&&, Type = STREAM_EQUAL_LINK);
	StreamEqualLink(const Handle&, const Handle&);

	StreamEqualLink(const StreamEqualLink&) = delete;
	StreamEqualLink& operator=(const StreamEqualLink&) = delete;

	virtual bool is_evaluatable() const { return true; }
	virtual bool bevaluate(AtomSpace*, bool);

	// Same as evaluation.
	virtual bool is_executable(void) const { return true; }
	virtual ValuePtr execute(AtomSpace* as, bool silent) {
		return ValueCast(evaluate(as, silent)); }

	static Handle factory(const Handle&);
};

LINK_PTR_DECL(StreamEqualLink)
#define createStreamEqualLink CREATE_DECL(StreamEqualLink)

/** @}*/
}

#endif // _OPENCOG_STREAM_EQUAL_LINK_H
