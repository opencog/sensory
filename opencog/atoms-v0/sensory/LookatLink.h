/*
 * opencog/atoms/sensory-v0/LookatLink.h
 *
 * Copyright (C) 2018,2024 Linas Vepstas
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

#ifndef _OPENCOG_LOOKAT_LINK_H
#define _OPENCOG_LOOKAT_LINK_H

#include <opencog/atoms/base/Link.h>
#include <opencog/sensory-v0/types/atom_types.h>

namespace opencog
{
/** \addtogroup grp_atomspace
 *  @{
 */

/// The LookatLink provides a description of a stream type.
///
class LookatLink : public Link
{
private:
	Type _kind;
	void init(void);

public:
	LookatLink(const HandleSeq&&, Type = LOOKAT_LINK);
	LookatLink(const Handle&);

	LookatLink(const LookatLink&) = delete;
	LookatLink& operator=(const LookatLink&) = delete;

	Type get_kind(void) { return _kind; }

	// Return a pointer to what was written.
	virtual ValuePtr execute(AtomSpace*, bool);
	virtual bool is_executable(void) const { return true; }

	static Handle factory(const Handle&);
};

LINK_PTR_DECL(LookatLink)
#define createLookatLink CREATE_DECL(LookatLink)

/** @}*/
}

#endif // _OPENCOG_LOOKAT_LINK_H
