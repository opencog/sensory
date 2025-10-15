/*
 * opencog/atoms/filedir/TextFileNode.cc
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
#include <sys/inotify.h>
#include <unistd.h>

#include <opencog/util/exceptions.h>
#include <opencog/util/oc_assert.h>
#include <opencog/atoms/base/Node.h>
#include <opencog/atoms/value/StringValue.h>
#include <opencog/atoms/value/VoidValue.h>
#include <opencog/atoms/value/ValueFactory.h>

#include <opencog/sensory/types/atom_types.h>
#include "TextFileNode.h"

using namespace opencog;

TextFileNode::TextFileNode(Type t, const std::string&& url) :
	TextStreamNode(t, std::move(url)),
	_fh(nullptr),
	_tail_mode(false),
	_inotify_fd(-1),
	_watch_fd(-1)
{
	OC_ASSERT(nameserver().isA(_type, TEXT_FILE_NODE),
		"Bad TextFileNode constructor!");
}

TextFileNode::TextFileNode(const std::string&& url) :
	TextStreamNode(TEXT_FILE_NODE, std::move(url)),
	_fh(nullptr),
	_tail_mode(false),
	_inotify_fd(-1),
	_watch_fd(-1)
{
}

TextFileNode::~TextFileNode()
{
	if (_watch_fd >= 0 && _inotify_fd >= 0)
		inotify_rm_watch(_inotify_fd, _watch_fd);
	if (_inotify_fd >= 0)
		::close(_inotify_fd);
	if (_fh)
		fclose(_fh);
}

/// Attempt to open the URL for writing.
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
///
/// Other possible extensions: this could also take configurable
/// parameters, via the (Predicate "*-some-parameter-*) message.
/// Such parameters could control flushing, appending vs clobbering,
/// tail mode, and so on. XXX TODO.

void TextFileNode::open(const ValuePtr& vty)
{
	TextStreamNode::open(vty);

	_fh = nullptr;
	const std::string& url = get_name();

	if (0 != url.compare(0, 8, "file:///"))
		throw RuntimeException(TRACE_INFO,
			"Unsupported URL \"%s\"\n", url.c_str());

	// Ignore the first 7 chars "file://"
	std::string pathstr = url.substr(7);
	const char* fpath = pathstr.c_str();
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

	// Setup inotify for tail mode
	if (_tail_mode)
	{
		_inotify_fd = inotify_init();
		if (_inotify_fd < 0)
		{
			int norr = errno;
			fclose(_fh);
			_fh = nullptr;
			throw RuntimeException(TRACE_INFO,
				"Failed to initialize inotify: %s\n", strerror(norr));
		}

		_watch_fd = inotify_add_watch(_inotify_fd, fpath, IN_MODIFY | IN_CLOSE_WRITE);
		if (_watch_fd < 0)
		{
			int norr = errno;
			::close(_inotify_fd);
			_inotify_fd = -1;
			fclose(_fh);
			_fh = nullptr;
			throw RuntimeException(TRACE_INFO,
				"Failed to add inotify watch on \"%s\": %s\n",
				fpath, strerror(norr));
		}
	}
}

void TextFileNode::close(const ValuePtr&)
{
	if (_watch_fd >= 0 && _inotify_fd >= 0)
	{
		inotify_rm_watch(_inotify_fd, _watch_fd);
		_watch_fd = -1;
	}
	if (_inotify_fd >= 0)
	{
		::close(_inotify_fd);
		_inotify_fd = -1;
	}
	if (_fh)
		fclose(_fh);
	_fh = nullptr;
	_tail_mode = false;
}

void TextFileNode::barrier(AtomSpace* ignore)
{
	if (_fh)
		fflush(_fh);
}

bool TextFileNode::connected(void) const
{
	return (nullptr != _fh);
}

// This will read one line from the text file, and return that line.
// This is a line-oriented, buffered interface.
// In tail mode, waits for new data using inotify when EOF is reached.
std::string TextFileNode::do_read(void) const
{
	static const std::string empty_string;

	// Not open. Can't do anything.
	if (nullptr == _fh) return empty_string;

#define BUFSZ 4096
	std::string str(BUFSZ, 0);
	char* buff = str.data();

	while (true)
	{
		char* rd = fgets(buff, BUFSZ, _fh);

		// Got data - return it
		if (nullptr != rd)
			return str;

		// Hit EOF
		if (!_tail_mode)
		{
			// Normal mode: close and end stream
			fclose(_fh);
			_fh = nullptr;
			return empty_string;
		}

		// Tail mode: wait for file modification
		clearerr(_fh);  // Clear EOF indicator

		// Wait for inotify event
		char event_buf[sizeof(struct inotify_event) + NAME_MAX + 1];
		ssize_t len = ::read(_inotify_fd, event_buf, sizeof(event_buf));

		if (len < 0)
		{
			if (errno == EINTR)
				continue;  // Interrupted, try again

			// Real error - close and return
			int norr = errno;
			fclose(_fh);
			_fh = nullptr;
			if (_watch_fd >= 0 && _inotify_fd >= 0)
			{
				inotify_rm_watch(_inotify_fd, _watch_fd);
				_watch_fd = -1;
			}
			if (_inotify_fd >= 0)
			{
				::close(_inotify_fd);
				_inotify_fd = -1;
			}
			throw RuntimeException(TRACE_INFO,
				"inotify read failed: %s\n", strerror(norr));
		}

		// File was modified - loop back and try reading again
	}
}

// ==============================================================
// Write stuff to a file.

void TextFileNode::do_write(const std::string& str)
{
	if (nullptr == _fh)
		throw RuntimeException(TRACE_INFO,
			"TextFile not open: URI \"%s\"\n", _name.c_str());

	fprintf(_fh, "%s", str.c_str());

	// flush, for now. This helps make the demos less confusing.
	// Future implementations could (should?) make this a configurable
	// parameter (using some (Predicate "*-config-*) style message.
	fflush(_fh);
}

// ==============================================================

// Adds factory when library is loaded.
DEFINE_NODE_FACTORY(TextFileNode, TEXT_FILE_NODE);

// ====================================================================

void opencog_sensory_filedir_init(void)
{
   // Force shared lib ctors to run
};
