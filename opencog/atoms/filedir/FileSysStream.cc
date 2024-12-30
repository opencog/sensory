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
#include <sys/stat.h>
#include <sys/types.h>

#include <opencog/util/exceptions.h>
#include <opencog/util/oc_assert.h>
#include <opencog/atomspace/AtomSpace.h>
#include <opencog/atoms/base/Link.h>
#include <opencog/atoms/base/Node.h>
#include <opencog/atoms/value/FloatValue.h>
#include <opencog/atoms/value/LinkValue.h>
#include <opencog/atoms/value/StringValue.h>
#include <opencog/atoms/value/ValueFactory.h>

#include <opencog/atoms/sensory-types/sensory_types.h>
#include "FileSysStream.h"

using namespace opencog;

/// Under construction.
/// The implementation here is attempting to follow the ideas sketched
/// in opencog/atoms/irc/README.md
///
/// Descriptions of a few basic connectors are given.
/// The rest have not been described.
///
/// There are half-a-dozen commands implemented, enough to
/// make this stream semi-usable in a pseudo "real-world" app.

// Get the URL string from the sensory node.
// In principle, `senso` should be a SensoryNode, but for the
// crrent round of demos, we relax this and accept anything
// that is a string. This should be reverted, someday.
static std::string get_url_string(const Handle& senso)
{
	ValuePtr relax = senso;
	if (senso->is_executable())
		relax = senso->execute();

	if (not relax->is_type(SENSORY_NODE))
		fprintf(stderr, "Caution: using relaxed stream creation spec %s\n",
			relax->to_string().c_str());

	if (relax->is_type(LINK_VALUE))
		relax = LinkValueCast(relax)->value()[0];

	if (relax->is_type(STRING_VALUE))
		return StringValueCast(relax)->value()[0];

	if (relax->is_node())
		return HandleCast(relax)->get_name();

	throw RuntimeException(TRACE_INFO,
		"Unable to get sring from %s\n", senso->to_string().c_str());
}

FileSysStream::FileSysStream(const Handle& sensor)
	: OutputStream(FILE_SYS_STREAM)
{
	// Get the URL now. Perhaps execution
	// can be or should be defered till later?
	init(get_url_string(sensor));
}

FileSysStream::FileSysStream(void)
	: OutputStream(FILE_SYS_STREAM)
{
	do_describe();
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

	_cwd = url;
	do_describe();

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

Handle _global_desc = Handle::UNDEFINED;

// TODO: The below lists only enough of the commands to do the
// basic wiring diagram work. There are additional commands,
// supported i an ad hoc manner, so that the motor demos can
// be developed. The motor demos are kind of independent of the
// wiring demos (for now).
void FileSysStream::do_describe(void)
{
	if (_global_desc) return;

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
						createNode(TYPE_NODE, "LinkValue"),
						createNode(TYPE_NODE, "StringValue")))));
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
						createNode(TYPE_NODE, "LinkValue"),
						createNode(TYPE_NODE, "StringValue")))));
	cmds.emplace_back(pwd_cmd);

	Handle cd_cmd =
		createLink(SECTION,
			createNode(ITEM_NODE, "the cd command"),
			createLink(CONNECTOR_SEQ,
				createLink(CONNECTOR,
					createNode(SEX_NODE, "command"),
					createNode(TYPE_NODE, "WriteLink")),
				createLink(CONNECTOR,
					createNode(SEX_NODE, "command"),
					createNode(ITEM_NODE, "cd")),
				createLink(CONNECTOR,
					createNode(SEX_NODE, "command"),
					createNode(TYPE_NODE, "SensoryNode")), // XXX maybe??
				createLink(CONNECTOR,
					createNode(SEX_NODE, "reply"),
					createLink(LINK_SIGNATURE_LINK,
						createNode(TYPE_NODE, "LinkValue"),
						createNode(TYPE_NODE, "StringValue")))));
	cmds.emplace_back(cd_cmd);

#ifdef LATER
	Handle mkdir_cmd =
		createLink(SECTION,
			createNode(ITEM_NODE, "the mkdir command"));
	cmds.emplace_back(mkdir_cmd);
#endif

	_global_desc = createLink(cmds, CHOICE_LINK);
}

// This is totally bogus because it is unused.
// This should be class static member
ValuePtr FileSysStream::describe(AtomSpace* as, bool silent)
{
	if (_description) return as->add_atom(_description);
	_description = as->add_atom(_global_desc);
	return _description;
}

// ==============================================================

// This will read one line from the file stream, and return that line.
// So, a line-oriented, buffered interface. For now.
void FileSysStream::update() const
{
	// if (nullptr == _fh) { _value.clear(); return; }
}

// ==============================================================
// helpers
static DIR* do_opendir(const std::string& path)
{
	DIR* dir = opendir(path.c_str());

	// XXX FIXME: for now, throw an error if the location cannot
	// be opened. Some better long-term architecture is desired.
	if (nullptr == dir)
	{
		int norr = errno;
		throw RuntimeException(TRACE_INFO,
			"Location %s inaccessible: %s",
			path.c_str(), strerror(norr));
	}
	return dir;
}


static ValuePtr make_stream_dirent(struct dirent* dent,
                                   const ValuePtr& locurl)
{
	std::string ftype = "unknown";
	switch (dent->d_type)
	{
		case DT_BLK: ftype = "block"; break;
		case DT_CHR: ftype = "char"; break;
		case DT_DIR: ftype = "dir"; break;
		case DT_FIFO: ftype = "fifo"; break;
		case DT_LNK: ftype = "lnk"; break;
		case DT_REG: ftype = "reg"; break;
		case DT_SOCK: ftype = "sock"; break;
		default: break;
	}
	ValueSeq vs({locurl});
	vs.emplace_back(createStringValue(ftype));
	return createLinkValue(vs);
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
	if (0 == cmd.compare("pwd"))
	{
		return createStringValue(_cwd);
	}

	// Commands without any arguments. These are applied to all
	// files/dirs in the current working dir.
	if (1 == cmdref->size())
	{
		const std::string& path = _cwd.substr(_pfxlen);
		DIR* dir = do_opendir(path.c_str());
		int fd = dirfd(dir);

		ValueSeq vents;
		struct dirent* dent = readdir(dir);
		for (; dent; dent = readdir(dir))
		{
			// Vague attempt to avoid infinite loops during recursion.
			// Directory listing is a recursive process, and recursively
			// descending into the "." directory is an infinite loop.
			// In principle, the agent should not do this. In practice,
			// the current agent architecture is not sophisticated
			// enough to handle this case cleanly. So, for now, as a
			// quick hack, disable the "." and ".." directories.
			// This is not enough for the general case, because
			// softlinks can create loops, and we follow softlinks.
			// Nor is this meant to be an inescapable gaol; soft links
			// might send us off into wild territories. For now, just
			// relax and go with the flow. We'll fix problems later.
			// XXX FIXME the problem above, later.
			if (0 == strcmp(dent->d_name, ".")) continue;
			if (0 == strcmp(dent->d_name, "..")) continue;

			ValuePtr locurl = createStringValue(_cwd + "/" + + dent->d_name);

			// Dispatch by command
			if (0 == cmd.compare("ls"))
			{
				vents.emplace_back(locurl);
				continue;
			}

			if (0 == cmd.compare("special"))
			{
				vents.emplace_back(make_stream_dirent(dent, locurl));
				continue;
			}

			// The remaining commands require performing a stat()
			unsigned int mask = STATX_BTIME | STATX_MTIME | STATX_SIZE;
			struct statx statxbuf;
			int rc = statx(fd, dent->d_name, 0, mask, &statxbuf);
			if (rc)
			{
				int norr = errno;
				closedir(dir);
				throw RuntimeException(TRACE_INFO,
					"Location %s error: %s",
					StringValueCast(locurl)->value()[0].c_str(),
					strerror(norr));
			}

			ValueSeq vs({locurl});
			if (0 == cmd.compare("btime"))
			{
				time_t epoch = statxbuf.stx_btime.tv_sec;
				vs.emplace_back(createStringValue(
					ctime(&epoch)));
				vents.emplace_back(createLinkValue(vs));
				continue;
			}

			if (0 == cmd.compare("mtime"))
			{
				time_t epoch = statxbuf.stx_mtime.tv_sec;
				vs.emplace_back(createStringValue(
					ctime(&epoch)));
				vents.emplace_back(createLinkValue(vs));
				continue;
			}

			if (0 == cmd.compare("filesize"))
			{
				vs.emplace_back(createFloatValue(
					(double) statxbuf.stx_size));
				vents.emplace_back(createLinkValue(vs));
				continue;
			}

			// If we are here, its an unknown commans
			closedir(dir);
			throw RuntimeException(TRACE_INFO,
				"Unknown command \"%s\"\n", cmd.c_str());
		}
		closedir(dir);
		return createLinkValue(vents);
	}

	// Commands taking a single argument; in all cases, it *must*.
	// be a file URL, presumably obtained previously with `ls`.
	cref = cmdref;
	if (not cref->is_link() or cref->size() != 2)
		throw RuntimeException(TRACE_INFO,
			"Expecting arguments; got %s", cref->to_string().c_str());

	const Handle& arg1 = cref->getOutgoingAtom(1);
	std::string fpath = get_url_string(arg1);

	if (fpath.compare(0, _pfxlen, _prefix))
		throw RuntimeException(TRACE_INFO,
			"Expecting file URL; got %s", arg1->to_string().c_str());

	if (0 == cmd.compare("cd"))
	{
		_cwd = fpath;
		return createStringValue(_cwd);
	}

	// Get dirent info for a single directory.
	// XXX borken for files. Needs fixin.
	if (0 == cmd.compare("special"))
	{
		const std::string& path = fpath.substr(_pfxlen);
		DIR* dir = do_opendir(path.c_str());

		ValueSeq vents;
		struct dirent* dent = readdir(dir);
		for (; dent; dent = readdir(dir))
		{
			if (strcmp(dent->d_name, ".")) continue;
			ValuePtr locurl = createStringValue(fpath);
			vents.emplace_back(make_stream_dirent(dent, locurl));
			break;
		}
		closedir(dir);
		return createLinkValue(vents);
	}

	if (0 == cmd.compare("magic"))
	{
	}

	throw RuntimeException(TRACE_INFO,
		"Unknown command \"%s\"\n", cmd.c_str());
}

// ==============================================================

// Adds factory when library is loaded.
DEFINE_VALUE_FACTORY(FILE_SYS_STREAM, createFileSysStream)
DEFINE_VALUE_FACTORY(FILE_SYS_STREAM, createFileSysStream, Handle)
