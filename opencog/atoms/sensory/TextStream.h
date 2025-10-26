/*
 * opencog/atoms/sensory/TextStream.h
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

#ifndef _OPENCOG_TEXT_STREAM_H
#define _OPENCOG_TEXT_STREAM_H

#include <stdio.h>
#include <opencog/atoms/sensory/SensoryNode.h>
#include <opencog/atoms/value/StringValue.h>

namespace opencog
{

/** \addtogroup grp_atomspace
 *  @{
 */

/**
 * TextStream wraps ObjectNodes that provide one-at-a-time *-read-*
 * methods, and provides a stream of string values obtained by reading,
 * on demand.
 *
 * This is similar to ReadStream but inherits from StringValue instead
 * of LinkValue, making it more suitable for text-oriented streaming.
 *
 * This is experimental.
 */
class TextStream
	: public StringValue
{
protected:
	SensoryNodePtr _snp;
	virtual void update() const;

public:
	TextStream(const Handle&);
	virtual ~TextStream();

	virtual std::string to_string(const std::string& indent = "") const;
};

VALUE_PTR_DECL(TextStream)
CREATE_VALUE_DECL(TextStream)

/** @}*/
} // namespace opencog

#endif // _OPENCOG_TEXT_STREAM_H
