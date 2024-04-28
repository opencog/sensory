/*
 * opencog/atoms/sensory/LookupLink.h
 *
 * Copyright (C) 2018 Linas Vepstas
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

#ifndef _LOOKUPCOG_LOOKUP_LINK_H
#define _LOOKUPCOG_LOOKUP_LINK_H

#include <opencog/atoms/base/Link.h>
#include <opencog/atoms/sensory-types/sensory_types.h>

namespace opencog
{
/** \addtogroup grp_atomspace
 *  @{
 */

/// The LookupLink provides a description of a stream type.
///
class LookupLink : public Link
{
private:
	Type _kind;
	void init(void);

public:
	LookupLink(const HandleSeq&&, Type = LOOKUP_LINK);
	LookupLink(const Handle&);

	LookupLink(const LookupLink&) = delete;
	LookupLink& operator=(const LookupLink&) = delete;

	Type get_kind(void) { return _kind; }

	// Return a pointer to what was written.
	virtual ValuePtr execute(AtomSpace*, bool);
	virtual bool is_executable(void) const { return true; }

	static Handle factory(const Handle&);
};

LINK_PTR_DECL(LookupLink)
#define createLookupLink CREATE_DECL(LookupLink)

/** @}*/
}

#endif // _LOOKUPCOG_LOOKUP_LINK_H
