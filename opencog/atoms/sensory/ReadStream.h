/*
 * opencog/atoms/sensory/ReadStream.h
 *
 * Copyright (C) 2024 Linas Vepstas
 * Copyright (C) 2025 BrainyBlaze Dynamics Inc.
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

#ifndef _OPENCOG_READ_STREAM_H
#define _OPENCOG_READ_STREAM_H

#include <stdio.h>
#include <opencog/atoms/sensory/SensoryNode.h>
#include <opencog/atoms/value/LinkStreamValue.h>

namespace opencog
{

/** \addtogroup grp_atomspace
 *  @{
 */

/**
 * ReadStream wraps ObjectNodes that provide one-at-a-time *-read-*
 * methods, and provides a stream of values obtained by reading,
 * on demand.
 *
 * This is experimental.
 */
class ReadStream
	: public LinkStreamValue
{
protected:
	SensoryNodePtr _snp;
	virtual void update() const;

public:
	ReadStream(const Handle&);
	virtual ~ReadStream();
};

VALUE_PTR_DECL(ReadStream)
CREATE_VALUE_DECL(ReadStream)

/** @}*/
} // namespace opencog

#endif // _OPENCOG_READ_STREAM_H
