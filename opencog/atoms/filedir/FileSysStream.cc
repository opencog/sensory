/*
 * opencog/atoms/sensory/FileSysStream.cc
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
#include "FileSysStream.h"

using namespace opencog;

FileSysStream::FileSysStream(const Handle& senso)
	: OutputStream(FILE_SYS_STREAM)
{
	if (SENSORY_NODE != senso->get_type())
		throw RuntimeException(TRACE_INFO,
			"Expecting SensoryNode, got %s\n", senso->to_string().c_str());

	init(senso->get_name());
}

FileSysStream::~FileSysStream()
{
	if (_fh)
		fclose (_fh);
}

void FileSysStream::init(const std::string& url)
{
	_fh = nullptr;
}

ValuePtr FileSysStream::describe(AtomSpace* as, bool silent)
{
	throw RuntimeException(TRACE_INFO, "Not implemeneted");
	return Handle::UNDEFINED;
}

// ==============================================================

// This will read one line from the file stream, and return that line.
// So, a line-oriented, buffered interface. For now.
void FileSysStream::update() const
{
	if (nullptr == _fh) { _value.clear(); return; }
}

// ==============================================================
// Write stuff to a file.

// Write stuff to a file.
ValuePtr FileSysStream::write_out(AtomSpace* as, bool silent,
                                   const Handle& cref)
{
	if (nullptr == _fh)
		throw RuntimeException(TRACE_INFO,
			"Text stream not open: URI \"%s\"\n", _uri.c_str());

	return createLinkValue();
}

// ==============================================================

// Adds factory when library is loaded.
DEFINE_VALUE_FACTORY(FILE_SYS_STREAM, createFileSysStream, Handle)
