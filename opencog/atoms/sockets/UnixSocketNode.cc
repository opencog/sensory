/*
 * opencog/atoms/unixsock/UnixSocketNode.cc
 *
 * Copyright (C) 2026 BrainyBlaze Dynamics LLC
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
 */

#include <errno.h>
#include <string.h> // for strerror()
#include <unistd.h>

#include <sys/socket.h>
#include <sys/un.h>

#include <opencog/util/exceptions.h>
#include <opencog/util/oc_assert.h>
#include <opencog/atoms/base/Node.h>
#include <opencog/atoms/value/StringValue.h>
#include <opencog/atoms/value/ValueFactory.h>

#include <opencog/sensory/types/atom_types.h>
#include "UnixSocketNode.h"

using namespace opencog;

UnixSocketNode::UnixSocketNode(Type t, const std::string&& url) :
	TextStreamNode(t, std::move(url)),
	_client_fd(-1),
	_listen_fd(-1)
{
	OC_ASSERT(nameserver().isA(_type, UNIX_SOCKET_NODE),
		"Bad UnixSocketNode constructor!");
}

UnixSocketNode::UnixSocketNode(const std::string&& url) :
	TextStreamNode(UNIX_SOCKET_NODE, std::move(url)),
	_client_fd(-1),
	_listen_fd(-1)
{
}

UnixSocketNode::~UnixSocketNode()
{
	// Clean up, if not already done.
	if (0 <= _client_fd)
		::close(_client_fd);
	_client_fd = -1;

	if (0 <= _listen_fd)
		::close(_listen_fd);
	_listen_fd = -1;

	// Remove the socket file from the filesystem.
	if (not _sock_path.empty())
		unlink(_sock_path.c_str());
}

/// Open a Unix domain socket and start listening for connections.
///
/// The URI format is:
/// unix:///path/to/socket
///
/// This creates the socket, binds to the given path, and starts
/// listening with a backlog of 1. It does NOT block waiting for a
/// client to connect; the accept() is deferred to the first call
/// to do_read() or do_write(). This allows the user to see the
/// socket file on disk and connect with socat at their leisure.
///
void UnixSocketNode::open(const ValuePtr& vty)
{
	TextStreamNode::open(vty);

	// If already listening or connected, do nothing.
	if (0 <= _client_fd or 0 <= _listen_fd) return;

	const std::string& url = get_name();

	if (0 != url.compare(0, 8, "unix:///"))
		throw RuntimeException(TRACE_INFO,
			"Unsupported URL \"%s\"\n"
			"Expected format: unix:///path/to/socket\n",
			url.c_str());

	// Extract the filesystem path. "unix://" is 7 chars.
	_sock_path = url.substr(7);

	// Remove any stale socket file left over from a previous run.
	unlink(_sock_path.c_str());

	// Create the Unix domain socket.
	_listen_fd = socket(AF_UNIX, SOCK_STREAM, 0);
	if (0 > _listen_fd)
		throw RuntimeException(TRACE_INFO,
			"Unable to create socket: (%d) %s\n",
			errno, strerror(errno));

	// Bind to the filesystem path.
	struct sockaddr_un addr;
	memset(&addr, 0, sizeof(addr));
	addr.sun_family = AF_UNIX;

	if (_sock_path.size() >= sizeof(addr.sun_path))
	{
		::close(_listen_fd);
		_listen_fd = -1;
		throw RuntimeException(TRACE_INFO,
			"Socket path too long (%zu bytes, max %zu): \"%s\"\n",
			_sock_path.size(), sizeof(addr.sun_path) - 1,
			_sock_path.c_str());
	}
	strncpy(addr.sun_path, _sock_path.c_str(), sizeof(addr.sun_path) - 1);

	if (0 > bind(_listen_fd, (struct sockaddr*)&addr, sizeof(addr)))
	{
		int norr = errno;
		::close(_listen_fd);
		_listen_fd = -1;
		throw RuntimeException(TRACE_INFO,
			"Unable to bind socket \"%s\": (%d) %s\n",
			_sock_path.c_str(), norr, strerror(norr));
	}

	// Listen with backlog of 1; we only handle one client.
	if (0 > listen(_listen_fd, 1))
	{
		int norr = errno;
		::close(_listen_fd);
		_listen_fd = -1;
		unlink(_sock_path.c_str());
		throw RuntimeException(TRACE_INFO,
			"Unable to listen on socket \"%s\": (%d) %s\n",
			_sock_path.c_str(), norr, strerror(norr));
	}

	printf("Listening on %s\n", _sock_path.c_str());
	printf("Connect with: socat - UNIX-CONNECT:%s\n", _sock_path.c_str());
}

/// Accept a client connection, if one has not yet been accepted.
/// This blocks until a client connects. Called lazily from do_read()
/// and do_write(). Caller must hold _mtx.
void UnixSocketNode::do_accept(void) const
{
	if (0 <= _client_fd) return;
	if (0 > _listen_fd) return;

	int cfd = accept(_listen_fd, nullptr, nullptr);
	if (0 > cfd)
	{
		int norr = errno;
		throw RuntimeException(TRACE_INFO,
			"Unable to accept connection on \"%s\": (%d) %s\n",
			_sock_path.c_str(), norr, strerror(norr));
	}

	printf("Client connected on %s\n", _sock_path.c_str());
	_client_fd = cfd;
}

void UnixSocketNode::close(const ValuePtr&)
{
	std::lock_guard<std::mutex> lock(_mtx);

	if (0 <= _client_fd)
		::close(_client_fd);
	_client_fd = -1;

	if (0 <= _listen_fd)
		::close(_listen_fd);
	_listen_fd = -1;

	_read_buf.clear();

	// Remove the socket file.
	if (not _sock_path.empty())
	{
		unlink(_sock_path.c_str());
		_sock_path.clear();
	}
}

void UnixSocketNode::barrier(AtomSpace* ignore)
{
	// Raw fd I/O has no user-space buffer to flush.
}

bool UnixSocketNode::connected(void) const
{
	return (0 <= _client_fd) or (0 <= _listen_fd);
}

// ==============================================================

// This will read one line from the socket, and return that line.
// This is a line-oriented, buffered interface.
// This blocks, waiting for input, if there is no input.
// On the first call, this will also block waiting for a client
// to connect (via accept()).
//
// Uses raw read() instead of fgets/FILE* to avoid stdio buffering
// issues on socket file descriptors.
std::string UnixSocketNode::do_read(void) const
{
	static const std::string empty_string;

	// If no client yet, accept one (blocks until a client connects).
	{
		std::lock_guard<std::mutex> lock(_mtx);
		if (0 > _client_fd)
		{
			do_accept();
			if (0 > _client_fd) return empty_string;
		}
	}

	// Check if the read buffer already contains a complete line.
	size_t nl = _read_buf.find('\n');
	if (nl != std::string::npos)
	{
		std::string line = _read_buf.substr(0, nl + 1);
		_read_buf.erase(0, nl + 1);
		return line;
	}

	// Read from the socket until we get a newline or EOF.
	char buf[4096];
	while (true)
	{
		int cfd;
		{
			std::lock_guard<std::mutex> lock(_mtx);
			cfd = _client_fd;
		}
		if (0 > cfd) return empty_string;

		ssize_t nr = ::read(cfd, buf, sizeof(buf));
		if (0 >= nr)
		{
			// EOF or error. Return whatever partial line we have,
			// or empty string if nothing buffered.
			if (_read_buf.empty())
				return empty_string;

			std::string line;
			line.swap(_read_buf);
			return line;
		}

		_read_buf.append(buf, nr);

		// Check for a complete line.
		nl = _read_buf.find('\n');
		if (nl != std::string::npos)
		{
			std::string line = _read_buf.substr(0, nl + 1);
			_read_buf.erase(0, nl + 1);
			return line;
		}
	}
}

// ==============================================================
// Write stuff to the socket.

void UnixSocketNode::do_write(const std::string& str)
{
	// If no client yet, accept one (blocks until a client connects).
	{
		std::lock_guard<std::mutex> lock(_mtx);
		if (0 > _client_fd)
			do_accept();
	}

	if (0 > _client_fd)
		throw RuntimeException(TRACE_INFO,
			"UnixSocket not open: URI \"%s\"\n", _name.c_str());

	// Write the entire string.
	const char* data = str.c_str();
	size_t remaining = str.length();
	while (0 < remaining)
	{
		ssize_t nw = ::write(_client_fd, data, remaining);
		if (0 > nw)
		{
			int norr = errno;
			if (EINTR == norr) continue;
			throw RuntimeException(TRACE_INFO,
				"Write error on socket \"%s\": (%d) %s\n",
				_sock_path.c_str(), norr, strerror(norr));
		}
		data += nw;
		remaining -= nw;
	}
}

// ==============================================================

// Adds factory when library is loaded.
DEFINE_NODE_FACTORY(UnixSocketNode, UNIX_SOCKET_NODE);

// ====================================================================

void opencog_sensory_sockets_init(void)
{
   // Force shared lib ctors to run
};
