/*
 * opencog/atoms/sensory/TextFileNode.cc
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
#include "TextFileNode.h"

using namespace opencog;

TextFileNode::TextFileNode(Type t, const std::string&& str)
	: SensoryNode(t, std::move(str))
{
	OC_ASSERT(nameserver().isA(_type, TEXT_FILE_NODE),
		"Bad TextFileNode constructor!");
	init();
}

TextFileNode::TextFileNode(const std::string&& str)
	: SensoryNode(TEXT_FILE_NODE, std::move(str))
{
	init();
}

TextFileNode::~TextFileNode()
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

void TextFileNode::init()
{
	_fresh = true;
	_fh = nullptr;
	const std::string& url = get_name();

	if (0 != url.compare(0, 8, "file:///"))
		throw RuntimeException(TRACE_INFO,
			"Unsupported URL \"%s\"\n", url.c_str());

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
void TextFileNode::open(const ValuePtr&)
{
}


void TextFileNode::close(const ValuePtr&)
{
}

void TextFileNode::write(const ValuePtr&)
{
}

bool TextFileNode::connected(void) const
{
	return true;
}

ValuePtr TextFileNode::read(void) const
{
	return createStringValue("foo");
}

// ==============================================================

ValuePtr TextFileNode::describe(AtomSpace* as, bool silent)
{
	throw RuntimeException(TRACE_INFO, "Not implemeneted");
	return Handle::UNDEFINED;
}

// ==============================================================
// Write stuff to a file.

void TextFileNode::do_write(const std::string& str)
{
	fprintf(_fh, "%s", str.c_str());
}

// Write stuff to a file.
ValuePtr TextFileNode::write_out(AtomSpace* as, bool silent,
                                   const Handle& cref)
{
	if (nullptr == _fh)
		throw RuntimeException(TRACE_INFO,
			"Text stream not open: URI \"%s\"\n", _name.c_str());

	return createStringValue("");
}

// ==============================================================

// Adds factory when library is loaded.
DEFINE_NODE_FACTORY(TextFileNode, TEXT_FILE_NODE);

// ====================================================================

void opencog_sensory_filedir_init(void)
{
   // Force shared lib ctors to run
};
