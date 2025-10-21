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

#include <opencog/util/exceptions.h>
#include <opencog/util/oc_assert.h>
#include <opencog/atoms/base/Node.h>
#include <opencog/atoms/value/BoolValue.h>
#include <opencog/atoms/value/StringValue.h>
#include <opencog/atoms/value/ValueFactory.h>

#include <opencog/sensory/types/atom_types.h>
#include "TextFileNode.h"

using namespace opencog;

TextFileNode::TextFileNode(Type t, const std::string&& url) :
	TextStreamNode(t, std::move(url)),
	_fh(nullptr),
	_tail_mode(false),
	_watcher()
{
	OC_ASSERT(nameserver().isA(_type, TEXT_FILE_NODE),
		"Bad TextFileNode constructor!");
}

TextFileNode::TextFileNode(const std::string&& url) :
	TextStreamNode(TEXT_FILE_NODE, std::move(url)),
	_fh(nullptr),
	_tail_mode(false),
	_watcher()
{
}

TextFileNode::~TextFileNode()
{
	_watcher.remove_watch();
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
		try
		{
			_watcher.add_watch(fpath);
		}
		catch (...)
		{
			fclose(_fh);
			_fh = nullptr;
			throw;
		}
	}
}

void TextFileNode::close(const ValuePtr&)
{
	std::lock_guard<std::mutex> lock(_mtx);
	_watcher.remove_watch();
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

void TextFileNode::follow(const ValuePtr& value)
{
	// Enable or disable tail mode based on BoolValue
	if (value->get_type() == BOOL_VALUE)
	{
		const std::vector<bool>& bv = BoolValueCast(value)->value();
		if (0 < bv.size())
		{
			bool new_mode = bv[0];

			// If enabling tail mode and file is open, set up watcher
			if (new_mode && !_tail_mode && _fh)
			{
				// Get the file path from the URL
				std::string url = get_name();
				std::string pathstr = url.substr(7); // Skip "file://"
				_watcher.add_watch(pathstr.c_str());
			}
			// If disabling tail mode, remove the watcher
			else if (!new_mode && _tail_mode)
			{
				_watcher.remove_watch();
			}

			_tail_mode = new_mode;
		}
	}
}

// This will read one line from the text file, and return that line.
// This is a line-oriented, buffered interface.
// In tail mode, waits for new data using inotify when EOF is reached.
std::string TextFileNode::do_read(void) const
{
	static const std::string empty_string;

	// Check if file is open (with lock)
	{
		std::lock_guard<std::mutex> lock(_mtx);
		if (nullptr == _fh) return empty_string;
	}

#define BUFSZ 4096
	std::string str(BUFSZ, 0);
	char* buff = str.data();

	while (true)
	{
		FILE* fh_copy;
		bool tail_mode_copy;

		// Lock to access _fh and read
		{
			std::lock_guard<std::mutex> lock(_mtx);

			// Check if closed while we were waiting
			if (nullptr == _fh) return empty_string;

			fh_copy = _fh;
			tail_mode_copy = _tail_mode;
		}

		// Read without holding lock (fgets is thread-safe for different FILEs)
		char* rd = fgets(buff, BUFSZ, fh_copy);

		// Got data - return it
		if (nullptr != rd)
		{
			// Resize string to actual length.
			str.resize(strlen(buff));
			return str;
		}

		// Hit EOF
		if (!tail_mode_copy)
		{
			// Normal mode: close and end stream
			std::lock_guard<std::mutex> lock(_mtx);
			if (_fh)  // Check again in case another thread closed it
			{
				fclose(_fh);
				_fh = nullptr;
			}
			return empty_string;
		}

		// Tail mode: wait for file modification
		clearerr(fh_copy);  // Clear EOF indicator

		// Wait for inotify event (WITHOUT holding lock - this blocks!)
		std::pair<uint32_t, std::string> event;
		try
		{
			event = _watcher.wait_event();
		}
		catch (...)
		{
			// Error - try to close and return
			std::lock_guard<std::mutex> lock(_mtx);
			if (_fh)
			{
				fclose(_fh);
				_fh = nullptr;
			}
			_watcher.remove_watch();
			throw;
		}

		// Check if watch was removed (shutdown signal from another thread)
		if (event.first == 0 && event.second.empty())
		{
			// Watch was closed - return empty to unblock
			return empty_string;
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
