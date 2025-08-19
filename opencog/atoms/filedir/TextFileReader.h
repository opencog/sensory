/*
 * opencog/atoms/filedir/TextFileReader.h
 *
 * Copyright (C) 2024 Linas Vepstas
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

#ifndef _OPENCOG_TEXT_FILE_READER_H
#define _OPENCOG_TEXT_FILE_READER_H

#include <stdio.h>
#include <opencog/atoms/value/LinkStreamValue.h>

namespace opencog
{

/** \addtogroup grp_atomspace
 *  @{
 */

/**
 * TextFileReader provides a stream of ItemNodes read from a text file,
 * and, more generally, from unix socket sources. This is experimental.
 */
class TextFileReader
	: public LinkStreamValue
{
protected:
	TextFileReader(Type t, const std::string&);
	void init(const std::string&);
	virtual void update() const;

	std::string _uri;
	mutable FILE* _fh;
	mutable bool _fresh;

public:
	TextFileReader(const Handle&);
	TextFileReader(const std::string&);
	virtual ~TextFileReader();
};

VALUE_PTR_DECL(TextFileReader)
CREATE_VALUE_DECL(TextFileReader)

/** @}*/
} // namespace opencog

#endif // _OPENCOG_TEXT_FILE_READER_H
