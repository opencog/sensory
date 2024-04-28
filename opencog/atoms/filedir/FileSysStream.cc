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

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h> // for strerror()
#include <sys/types.h>

#include <opencog/util/exceptions.h>
#include <opencog/util/oc_assert.h>
#include <opencog/atomspace/AtomSpace.h>
#include <opencog/atoms/base/Link.h>
#include <opencog/atoms/base/Node.h>
#include <opencog/atoms/value/LinkValue.h>
#include <opencog/atoms/value/StringValue.h>
#include <opencog/atoms/value/ValueFactory.h>

#include <opencog/atoms/sensory-types/sensory_types.h>
#include "FileSysStream.h"

using namespace opencog;

/// Under construction.
/// The implementation here is attempting to follow the ideas sketched
/// in opencog/atoms/irc/README.md

FileSysStream::FileSysStream(const Handle& senso)
	: OutputStream(FILE_SYS_STREAM)
{
	if (SENSORY_NODE != senso->get_type())
		throw RuntimeException(TRACE_INFO,
			"Expecting SensoryNode, got %s\n", senso->to_string().c_str());

	init(senso->get_name());
}

FileSysStream::FileSysStream(void)
	: OutputStream(FILE_SYS_STREAM)
{
}

FileSysStream::~FileSysStream()
{
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

static const std::string _prefix("file://");
static const size_t _pfxlen = _prefix.size();

void FileSysStream::init(const std::string& url)
{
	if (0 != url.compare(0, 8, "file:///"))
		throw RuntimeException(TRACE_INFO,
			"Unsupported URL \"%s\"\n", url.c_str());

	// Make a copy, for debuggingg purposes.
	_uri = url;

#if LATER
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
#endif
}

// ==============================================================

// This is totally bogus because it is unused.
// This should be class static member
ValuePtr FileSysStream::describe(AtomSpace* as, bool silent)
{
	if (_description) return as->add_atom(_description);
	HandleSeq cmds;

	// List files
	Handle ls_cmd =
		createLink(SECTION,
			createNode(ITEM_NODE, "the ls command"),
			createLink(CONNECTOR_SEQ,
				createLink(CONNECTOR,
					createNode(SEX_NODE, "command"),
					createNode(TYPE_NODE, "WriteLink")),
				createLink(CONNECTOR,
					createNode(SEX_NODE, "command"),
					createNode(ITEM_NODE, "ls")),
				createLink(CONNECTOR,
					createNode(SEX_NODE, "reply"),
					createLink(LINK_SIGNATURE_LINK,
						createNode(TYPE_NODE, "LinkValue")),
						createNode(TYPE_NODE, "StringValue"))));
	cmds.emplace_back(ls_cmd);

	Handle pwd_cmd =
		createLink(SECTION,
			createNode(ITEM_NODE, "the pwd command"),
			createLink(CONNECTOR_SEQ,
				createLink(CONNECTOR,
					createNode(SEX_NODE, "command"),
					createNode(TYPE_NODE, "WriteLink")),
				createLink(CONNECTOR,
					createNode(SEX_NODE, "command"),
					createNode(ITEM_NODE, "pwd")),
				createLink(CONNECTOR,
					createNode(SEX_NODE, "reply"),
					createLink(LINK_SIGNATURE_LINK,
						createNode(TYPE_NODE, "LinkValue")),
						createNode(TYPE_NODE, "StringValue"))));
	cmds.emplace_back(pwd_cmd);

	Handle cd_cmd =
		createLink(SECTION,
			createNode(ITEM_NODE, "the cd command"));
	cmds.emplace_back(cd_cmd);

	Handle mkdir_cmd =
		createLink(SECTION,
			createNode(ITEM_NODE, "the mkdir command"));
	cmds.emplace_back(mkdir_cmd);

	_description = createLink(cmds, CHOICE_LINK);
	return as->add_atom(_description);
}

// ==============================================================

// This will read one line from the file stream, and return that line.
// So, a line-oriented, buffered interface. For now.
void FileSysStream::update() const
{
	// if (nullptr == _fh) { _value.clear(); return; }
}

// ==============================================================

// Process a command.
ValuePtr FileSysStream::write_out(AtomSpace* as, bool silent,
                                  const Handle& cmdref)
{
	Handle cref = cmdref;
	if (cref->is_link())
	{
		if (0 == cref->size())
			throw RuntimeException(TRACE_INFO,
				"Expecting a non-empty list: %s", cref->to_string().c_str());
		cref = cref->getOutgoingAtom(0);
	}
	if (not cref->is_node())
		throw RuntimeException(TRACE_INFO,
			"Expecting a Node: %s", cref->to_string().c_str());

	const std::string& cmd = cref->get_name();
	if (0 == cmd.compare("ls"))
	{
		const std::string& path = _uri.substr(_pfxlen);
		DIR* dir = opendir(path.c_str());

		ValueSeq vents;
		struct dirent* dent = readdir(dir);
		while (dent)
		{
			vents.emplace_back(createStringValue(_uri + "/" + + dent->d_name));
			dent = readdir(dir);
		}
		closedir(dir);
		return createLinkValue(vents);
	}

	if (0 == cmd.compare("pwd"))
	{
		return createStringValue(_uri);
	}

	// Commands taking arguments
	cref = cmdref;
	if (not cref->is_link() or cref->size() < 2)
		throw RuntimeException(TRACE_INFO,
			"Expecting arguments: %s", cref->to_string().c_str());
	const Handle& arg1 = cref->getOutgoingAtom(1);

	if (0 == cmd.compare("cd"))
	{
		if (not arg1->is_node())
			throw RuntimeException(TRACE_INFO,
				"Expecting filepath: %s", arg1->to_string().c_str());

		const std::string& fpath = arg1->get_name();
		if (fpath.compare(0, _pfxlen, _prefix))
			throw RuntimeException(TRACE_INFO,
				"Expecting filepath: %s", arg1->to_string().c_str());

		_uri = fpath;
		return createStringValue(_uri);
	}

	throw RuntimeException(TRACE_INFO,
		"Unknown command \"%s\"\n", cmd.c_str());
}

// ==============================================================

// Adds factory when library is loaded.
DEFINE_VALUE_FACTORY(FILE_SYS_STREAM, createFileSysStream)
DEFINE_VALUE_FACTORY(FILE_SYS_STREAM, createFileSysStream, Handle)
