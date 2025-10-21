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
#include <limits.h>
#include <poll.h>
#include <string.h>
#include <sys/inotify.h>
#include <unistd.h>

#include <opencog/util/exceptions.h>
#include <opencog/atoms/value/StringValue.h>
#include <opencog/atoms/value/ValueFactory.h>

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
	stop_watching();
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
	std::lock_guard<std::mutex> lock(_mtx);

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
	std::lock_guard<std::mutex> lock(_mtx);
	cleanup_watch();
	cleanup_inotify();
}

std::pair<uint32_t, std::string> FileWatcher::wait_event()
{
	int inotify_fd_copy;

	// Check if watch is active (with lock)
	{
		std::lock_guard<std::mutex> lock(_mtx);
		if (_inotify_fd < 0 || _watch_fd < 0)
		{
			throw RuntimeException(TRACE_INFO,
				"FileWatcher::wait_event() called without active watch\n");
		}
		inotify_fd_copy = _inotify_fd;
	}

	// Buffer for inotify events
	// Must be large enough for at least one event plus the largest possible filename
	char event_buf[sizeof(struct inotify_event) + NAME_MAX + 1];

	while (true)
	{
		// Read without holding lock (this blocks)
		ssize_t len = ::read(inotify_fd_copy, event_buf, sizeof(event_buf));

		if (len < 0)
		{
			// Interrupted by signal or would block (non-blocking mode) - retry
			if (errno == EINTR || errno == EAGAIN)
				continue;

			// If fd was closed from another thread (during shutdown), return gracefully
			// This happens when another thread calls remove_watch() while we're blocked
			if (errno == EBADF || errno == EINVAL)
			{
				// Return a sentinel value indicating watch was removed
				return std::make_pair(0, std::string());
			}

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

bool FileWatcher::poll_and_add_events(const ContainerValuePtr& cvp, int timeout_ms)
{
	int inotify_fd_copy;

	// Check if watch is active (with lock)
	{
		std::lock_guard<std::mutex> lock(_mtx);
		if (_inotify_fd < 0 || _watch_fd < 0)
			return false; // Not watching, signal exit
		inotify_fd_copy = _inotify_fd;
	}

	// Poll for events (without holding lock)
	struct pollfd pfd;
	pfd.fd = inotify_fd_copy;
	pfd.events = POLLIN;

	int ret = poll(&pfd, 1, timeout_ms);
	if (ret < 0)
	{
		if (errno == EINTR)
			return true; // Interrupted, continue watching
		return false; // Error, signal exit
	}

	if (ret == 0)
		return true; // Timeout, continue watching

	// Read inotify events (without holding lock)
	char buf[4096] __attribute__((aligned(__alignof__(struct inotify_event))));
	ssize_t len = read(inotify_fd_copy, buf, sizeof(buf));
	if (len < 0)
	{
		if (errno == EAGAIN || errno == EINTR)
			return true; // Would block or interrupted, continue watching
		return false; // Error or fd closed, signal exit
	}

	if (len == 0)
		return true; // No data, continue watching

	// Process events and add to container
	const struct inotify_event *event;
	for (char *ptr = buf; ptr < buf + len;
	     ptr += sizeof(struct inotify_event) + event->len)
	{
		event = (const struct inotify_event *) ptr;

		// Check for overflow (ignore as requested)
		if (event->mask & IN_Q_OVERFLOW)
			continue;

		// Only process events with filenames
		if (event->len > 0)
		{
			std::string filename(event->name);
			ValuePtr vp = createStringValue(filename);
			cvp->add(vp);
		}
	}

	return true; // Events processed, continue watching
}

void FileWatcher::watch_thread_func(const ContainerValuePtr& cvp, int timeout_ms)
{
	// Poll and add events in a loop until told to stop
	while (poll_and_add_events(cvp, timeout_ms))
	{
		// Continue watching...
	}
	// When poll_and_add_events returns false, exit thread
}

void FileWatcher::start_watching(const std::string& path, const ContainerValuePtr& cvp, int timeout_ms)
{
	{
		std::lock_guard<std::mutex> lock(_mtx);
		// Check if already watching
		if (_watch_thread.joinable())
			throw RuntimeException(TRACE_INFO,
				"FileWatcher already watching - call stop_watching() first\n");
	}

	// Setup watch (add_watch has its own lock)
	add_watch(path);

	// Start background thread
	{
		std::lock_guard<std::mutex> lock(_mtx);
		_watch_thread = std::thread(&FileWatcher::watch_thread_func, this, cvp, timeout_ms);
	}
}

void FileWatcher::stop_watching()
{
	bool should_join = false;

	// Check if thread is running and cleanup (with lock)
	{
		std::lock_guard<std::mutex> lock(_mtx);
		if (_watch_thread.joinable())
		{
			should_join = true;
			cleanup_watch();     // This will cause poll_and_add_events to return false
			cleanup_inotify();   // Close inotify fd
		}
	}

	// Join thread without holding lock (join can block)
	if (should_join)
		_watch_thread.join();
}
