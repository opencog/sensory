/*
 * opencog/atoms/sensory/WriteLink.h
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

#ifndef _OPENCOG_WRITE_LINK_H
#define _OPENCOG_WRITE_LINK_H

#include <opencog/atoms/base/Link.h>
#include <opencog/atoms/sensory-types/sensory_types.h>

namespace opencog
{
/** \addtogroup grp_atomspace
 *  @{
 */

/// The WriteLink moves data from Atoms to some output stream.
///
class WriteLink : public Link
{
private:
	void init(void);

public:
	WriteLink(const HandleSeq&&, Type = WRITE_LINK);
	WriteLink(const Handle&);

	WriteLink(const WriteLink&) = delete;
	WriteLink& operator=(const WriteLink&) = delete;

	// Return a pointer to what was written.
	virtual ValuePtr execute(AtomSpace*, bool);
	virtual bool is_executable(void) { return true; }

	static Handle factory(const Handle&);
};

LINK_PTR_DECL(WriteLink)
#define createWriteLink CREATE_DECL(WriteLink)

/** @}*/
}

#endif // _OPENCOG_WRITE_LINK_H
