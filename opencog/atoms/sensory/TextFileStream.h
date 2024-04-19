/*
 * opencog/atoms/sensory/TextFileStream.h
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

#ifndef _OPENCOG_TEXT_FILE_STREAM_H
#define _OPENCOG_TEXT_FILE_STREAM_H

#include <stdio.h>
#include <opencog/atoms/sensory/OutputStream.h>

namespace opencog
{

/** \addtogroup grp_atomspace
 *  @{
 */

/**
 * TextFileStreams provide a stream of PhraseNodes read from from
 * a text file and, more generally, from unix socket sources. This
 * is experimental.
 */
class TextFileStream
	: public OutputStream
{
protected:
	TextFileStream(Type t, const std::string&);
	void init(const std::string&);
	virtual void update() const;

	FILE* _fh;

public:
	TextFileStream(const std::string&);
	virtual ~TextFileStream();

	virtual ValuePtr write_out(const Handle&);
	virtual bool operator==(const Value&) const;
};

typedef std::shared_ptr<TextFileStream> TextFileStreamPtr;
static inline TextFileStreamPtr TextFileStreamCast(ValuePtr& a)
	{ return std::dynamic_pointer_cast<TextFileStream>(a); }

template<typename ... Type>
static inline std::shared_ptr<TextFileStream> createTextFileStream(Type&&... args) {
   return std::make_shared<TextFileStream>(std::forward<Type>(args)...);
}

/** @}*/
} // namespace opencog

#endif // _OPENCOG_TEXT_FILE_STREAM_H
