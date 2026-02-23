/*
 * opencog/atoms/sensory/UnixSocketNode.h
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

#ifndef _OPENCOG_UNIX_SOCKET_NODE_H
#define _OPENCOG_UNIX_SOCKET_NODE_H

#include <stdio.h>
#include <mutex>
#include <opencog/atoms/sensory/TextStreamNode.h>

namespace opencog
{

/** \addtogroup grp_atomspace
 *  @{
 */

/**
 * UnixSocketNodes provide a stream of StringValues read from a Unix
 * domain socket, and written to it. This allows interactive text I/O
 * with an external process, e.g. a human user running `socat` to
 * attach to the socket.
 *
 * The node acts as a server: on open, it creates, binds, and listens
 * on the socket, and then waits for a client to connect. Once
 * connected, line-oriented text can be read and written.
 *
 * The close/read interaction is thread safe: the only way to break
 * out of a blocking read in one thread is to call close() from a
 * different thread.
 *
 * URI format: unix:///path/to/socket
 *
 * This is experimental.
 * Unsolved issues:
 * -- Fails to handle lines longer than 4096
 * -- Accepts only one client at a time.
 */
class UnixSocketNode
	: public TextStreamNode
{
protected:
	mutable std::mutex _mtx;  // Protects _fh and coordinates close/read
	mutable FILE* _fh;        // FILE* for the accepted client connection
	int _listen_fd;            // Listening socket file descriptor
	std::string _sock_path;   // Filesystem path to the socket

	virtual void do_write(const std::string&);

	virtual void open(const ValuePtr&);
	virtual void close(const ValuePtr&);
	// virtual void write(const ValuePtr&); inherited from StreamNode
	virtual bool connected(void) const;
	virtual void barrier(AtomSpace* = nullptr);
	virtual std::string do_read(void) const;

public:
	UnixSocketNode(const std::string&&);
	UnixSocketNode(Type t, const std::string&&);
	virtual ~UnixSocketNode();

	static Handle factory(const Handle&);
};

NODE_PTR_DECL(UnixSocketNode)
#define createUnixSocketNode CREATE_DECL(UnixSocketNode)

/** @}*/
} // namespace opencog

extern "C" {
void opencog_sensory_unixsock_init(void);
};

#endif // _OPENCOG_UNIX_SOCKET_NODE_H
