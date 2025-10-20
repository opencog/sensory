/*
 * opencog/atoms/filedir/FileWatcher.cc
 *
 * Copyright (C) 2025 BrainyBlaze Dynamics, Inc.
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
#include <string.h>
#include <sys/inotify.h>
#include <unistd.h>
#include <limits.h>

#include <opencog/util/exceptions.h>

#include "FileWatcher.h"

using namespace opencog;

FileWatcher::FileWatcher() :
	_inotify_fd(-1),
	_watch_fd(-1),
	_watch_path(),
	_event_mask(0)
{
}

FileWatcher::~FileWatcher()
{
	cleanup_watch();
	cleanup_inotify();
}

void FileWatcher::cleanup_watch()
{
	if (_watch_fd >= 0 && _inotify_fd >= 0)
	{
		inotify_rm_watch(_inotify_fd, _watch_fd);
		_watch_fd = -1;
	}
	_watch_path.clear();
	_event_mask = 0;
}

void FileWatcher::cleanup_inotify()
{
	if (_inotify_fd >= 0)
	{
		::close(_inotify_fd);
		_inotify_fd = -1;
	}
}

void FileWatcher::add_watch(const std::string& path)
{
	// Remove any existing watch first
	if (_watch_fd >= 0)
		cleanup_watch();

	// Initialize inotify if not already done
	if (_inotify_fd < 0)
	{
		_inotify_fd = inotify_init1(IN_NONBLOCK);
		if (_inotify_fd < 0)
		{
			int norr = errno;
			throw RuntimeException(TRACE_INFO,
				"Failed to initialize inotify: %s\n", strerror(norr));
		}
	}

	// Event mask for watching files and directories
	uint32_t mask = IN_CREATE | IN_MODIFY | IN_MOVED_TO | IN_DELETE | IN_CLOSE_WRITE;

	// Add watch on the path
	_watch_fd = inotify_add_watch(_inotify_fd, path.c_str(), mask);
	if (_watch_fd < 0)
	{
		int norr = errno;
		cleanup_inotify();
		throw RuntimeException(TRACE_INFO,
			"Failed to add inotify watch on \"%s\": %s\n",
			path.c_str(), strerror(norr));
	}

	_watch_path = path;
	_event_mask = mask;
}

void FileWatcher::remove_watch()
{
	cleanup_watch();
	cleanup_inotify();
}

std::pair<uint32_t, std::string> FileWatcher::wait_event()
{
	if (_inotify_fd < 0 || _watch_fd < 0)
	{
		throw RuntimeException(TRACE_INFO,
			"FileWatcher::wait_event() called without active watch\n");
	}

	// Buffer for inotify events
	// Must be large enough for at least one event plus the largest possible filename
	char event_buf[sizeof(struct inotify_event) + NAME_MAX + 1];

	while (true)
	{
		ssize_t len = ::read(_inotify_fd, event_buf, sizeof(event_buf));

		if (len < 0)
		{
			// Interrupted by signal or would block (non-blocking mode) - retry
			if (errno == EINTR || errno == EAGAIN)
				continue;

			// Real error
			int norr = errno;
			throw RuntimeException(TRACE_INFO,
				"inotify read failed: %s\n", strerror(norr));
		}

		// Parse the event
		struct inotify_event* event = (struct inotify_event*)event_buf;

		// Extract filename if present (for directory watches)
		std::string filename;
		if (event->len > 0)
			filename = std::string(event->name);

		return std::make_pair(event->mask, filename);
	}
}
