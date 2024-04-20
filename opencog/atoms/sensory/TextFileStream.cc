/*
 * opencog/atoms/sensory/TextFileStream.cc
 *
 * Copyright (C) 2020 Linas Vepstas
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

#include <errno.h>
#include <string.h> // for strerror()

#include <opencog/util/exceptions.h>
#include <opencog/util/oc_assert.h>
#include <opencog/atoms/base/Node.h>
#include <opencog/atoms/value/StringValue.h>
#include <opencog/atoms/value/ValueFactory.h>

#include <opencog/atoms/sensory-types/sensory_types.h>
#include "TextFileStream.h"

using namespace opencog;

TextFileStream::TextFileStream(Type t, const std::string& str)
	: OutputStream(t)
{
	OC_ASSERT(nameserver().isA(_type, TEXT_FILE_STREAM),
		"Bad TextFileStream constructor!");
	init(str);
}

TextFileStream::TextFileStream(const std::string& str)
	: OutputStream(TEXT_FILE_STREAM)
{
	init(str);
}

TextFileStream::~TextFileStream()
{
}

/// Attempt to open the URL for reading.
/// The URL format is described in
/// https://en.wikipedia.org/wiki/File_URI_scheme
/// and we adhere to that.
///
/// URI formats are:
/// file:/path       ; Not currently supported
/// file:///path     ; Yes, use this
/// file://host/path ; Not currently supported
/// file://./path    ; Dot means localhost
///
/// Possible extensions:
/// file:mode//...
/// where mode is one of the modes described in `man 3 fopen`

void TextFileStream::init(const std::string& url)
{
	_fh = nullptr;
	if (0 != url.compare(0, 8, "file:///"))
		throw RuntimeException(TRACE_INFO,
			"Unsupported URL \"%s\"\n", url.c_str());

	// Make a copy, for debuggingg purposes.
	_uri = url;

	// Ignore the first 7 chars "file://"
	const char* fpath = url.substr(7).c_str();
	_fh = fopen(fpath, "a+");

	if (nullptr == _fh)
	{
		int norr = errno;
		char buff[80];
		buff[0] = 0;
		// Apparently, we are getting the Gnu version of strerror_r
		// and not the XSI version. I suppose it doesn't matter.
		char * ers = strerror_r(norr, buff, 80);
		throw RuntimeException(TRACE_INFO,
			"Unable to open URL \"%s\"\nError was \"%s\"\n",
			url.c_str(), ers);
	}
}

// ==============================================================

// This will read one line from the file stream, and return that line.
// So, a line-oriented, buffered interface. For now.
void TextFileStream::update() const
{
	if (nullptr == _fh) { _value.clear(); return; }

#define BUFSZ 4080
	char buff[BUFSZ];
	char* rd = fgets(buff, BUFSZ, _fh);
	if (nullptr == rd)
	{
		// fclose(_fh);
		// _fh = nullptr;
		_value.clear();
		 return;
	}

	_value.resize(1);
	_value[0] = createNode(ITEM_NODE, buff);
}

// ==============================================================
// Write stuff to a file.
void TextFileStream::prt_value(const ValuePtr& content)
{
	if (content->is_type(STRING_VALUE))
	{
		StringValuePtr svp(StringValueCast(content));
		const std::vector<std::string>& strs = svp->value();
		for (const std::string& str : strs)
			fprintf(_fh, "%s", str.c_str());
		return;
	}
	if (content->is_type(NODE))
	{
		const std::string& name = HandleCast(content)->get_name();
		fprintf(_fh, " %s", name.c_str());
		return;
	}
	if (content->is_type(LINK_VALUE))
	{
		LinkValuePtr lvp(LinkValueCast(content));
		const ValueSeq& vals = lvp->value();
		for (const ValuePtr& v : vals)
			prt_value(v);
		return;
	}

	throw RuntimeException(TRACE_INFO,
		"Expecting strings, got %s\n", content->to_string().c_str());
}

// Write stuff to a file.
ValuePtr TextFileStream::write_out(AtomSpace* as, bool silent,
                                   const Handle& cref)
{
	if (nullptr == _fh)
		throw RuntimeException(TRACE_INFO,
			"Text stream not open: URI \"%s\"\n", _uri.c_str());

	ValuePtr content = cref;
	if (cref->is_executable())
	{
		content = cref->execute(as, silent);
		if (nullptr == content)
			throw RuntimeException(TRACE_INFO,
				"Expecting something to write from %s\n",
				cref->to_string().c_str());
	}

	// If it is a stream, enter infinite loop, until it is exahausted.
	if (content->is_type(LINK_STREAM_VALUE))
	{
		LinkValuePtr lvp(LinkValueCast(content));
		while (true)
		{
			const ValueSeq& vals = lvp->value();
			if (0 == vals.size()) break;
			for (const ValuePtr& v : vals)
				prt_value(v);
			fflush(_fh);
		}
	}

	prt_value(content);
	fflush(_fh);
	return content;
}

// ==============================================================

bool TextFileStream::operator==(const Value& other) const
{
	// Derived classes use this, so use get_type()
	if (get_type() != other.get_type()) return false;

	if (this == &other) return true;

	return LinkValue::operator==(other);
}

// ==============================================================

// Adds factory when library is loaded.
DEFINE_VALUE_FACTORY(TEXT_FILE_STREAM, createTextFileStream, std::string)
