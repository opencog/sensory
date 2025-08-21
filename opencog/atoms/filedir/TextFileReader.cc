/*
 * opencog/atoms/filedir/TextFileReader.cc
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

#include <opencog/sensory/types/atom_types.h>
#include "TextFileReader.h"

using namespace opencog;

TextFileReader::TextFileReader(Type t, const std::string& str)
	: LinkStreamValue(t)
{
	OC_ASSERT(nameserver().isA(_type, TEXT_FILE_READER_STREAM),
		"Bad TextFileReader constructor!");
	init(str);
}

TextFileReader::TextFileReader(const std::string& str)
	: LinkStreamValue(TEXT_FILE_READER_STREAM)
{
	init(str);
}

TextFileReader::TextFileReader(const Handle& senso)
	: LinkStreamValue(TEXT_FILE_READER_STREAM)
{
	if (SENSORY_NODE != senso->get_type())
		throw RuntimeException(TRACE_INFO,
			"Expecting SensoryNode, got %s\n", senso->to_string().c_str());

	init(senso->get_name());
}

TextFileReader::~TextFileReader()
{
	if (_fh)
		fclose (_fh);
}

/// Attempt to open the URL for reading and writing.
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

void TextFileReader::init(const std::string& url)
{
	_fresh = true;
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
void TextFileReader::update() const
{
	if (nullptr == _fh) { _value.clear(); return; }

	// The very first call after opening a file will typically
	// be a bogus update, so as to give the caller something,
	// anything. There will be trouble down the line, when
	// actually reading. So first time through, return the URL
	if (_fresh)
	{
		_fresh = false;
		_value.resize(1);
		_value[0] = createNode(ITEM_NODE, _uri);
		return;
	}

#define BUFSZ 4080
	char buff[BUFSZ];
	char* rd = fgets(buff, BUFSZ, _fh);
	if (nullptr == rd)
	{
		fclose(_fh);
		_fh = nullptr;
		_value.clear();
		return;
	}

	_value.resize(1);
	_value[0] = createNode(ITEM_NODE, buff);
}

// ==============================================================

// Adds factory when library is loaded.
DEFINE_VALUE_FACTORY(TEXT_FILE_READER_STREAM, createTextFileReader, std::string)
DEFINE_VALUE_FACTORY(TEXT_FILE_READER_STREAM, createTextFileReader, Handle)

// ====================================================================
