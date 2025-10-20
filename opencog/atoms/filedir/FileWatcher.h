/*
 * opencog/atoms/filedir/FileWatcher.h
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

#ifndef _OPENCOG_FILE_WATCHER_H
#define _OPENCOG_FILE_WATCHER_H

#include <string>
#include <utility>

namespace opencog
{
/** \addtogroup grp_atomspace
 *  @{
 */

/**
 * FileWatcher - Abstraction for inotify-based file and directory watching
 *
 * This class encapsulates the common inotify functionality needed for
 * monitoring files and directories for changes. It manages the inotify
 * file descriptors, watch setup/teardown, and event reading.
 *
 * Usage pattern:
 *   FileWatcher watcher;
 *   watcher.add_watch("/path/to/file", IN_MODIFY | IN_CLOSE_WRITE);
 *
 *   while (true) {
 *       auto [event_mask, filename] = watcher.wait_event();
 *       // Process event
 *   }
 *
 *   watcher.remove_watch();
 */
class FileWatcher
{
private:
	int _inotify_fd;
	int _watch_fd;
	std::string _watch_path;
	uint32_t _event_mask;

	void cleanup_watch();
	void cleanup_inotify();

public:
	FileWatcher();
	~FileWatcher();

	// Prevent copying
	FileWatcher(const FileWatcher&) = delete;
	FileWatcher& operator=(const FileWatcher&) = delete;

	/**
	 * Add a watch on the specified path.
	 *
	 * @param path The file or directory path to watch
	 * @throws RuntimeException if inotify initialization or watch setup fails
	 */
	void add_watch(const std::string& path);

	/**
	 * Remove the current watch and cleanup.
	 */
	void remove_watch();

	/**
	 * Wait for the next inotify event (blocking).
	 *
	 * @return A pair of (event_mask, filename) where:
	 *         - event_mask: The inotify event that occurred
	 *         - filename: The name of the file (for directory watches)
	 *                    or empty string (for file watches)
	 * @throws RuntimeException if inotify read fails (non-EINTR error)
	 */
	std::pair<uint32_t, std::string> wait_event();

	/**
	 * Check if currently watching a path.
	 *
	 * @return true if a watch is active, false otherwise
	 */
	bool is_watching() const { return _watch_fd >= 0; }

	/**
	 * Get the currently watched path.
	 *
	 * @return The path being watched, or empty string if not watching
	 */
	const std::string& watched_path() const { return _watch_path; }

	/**
	 * Get the inotify file descriptor.
	 * Useful for using with poll() or select() in custom event loops.
	 *
	 * @return The inotify file descriptor, or -1 if not initialized
	 */
	int get_fd() const { return _inotify_fd; }
};

/** @}*/
} // namespace opencog

#endif // _OPENCOG_FILE_WATCHER_H
