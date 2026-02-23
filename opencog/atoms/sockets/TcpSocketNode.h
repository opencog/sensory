/*
 * opencog/atoms/tcpsock/TcpSocketNode.h
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

#ifndef _OPENCOG_TCP_SOCKET_NODE_H
#define _OPENCOG_TCP_SOCKET_NODE_H

#include <mutex>
#include <opencog/atoms/sensory/TextStreamNode.h>

namespace opencog
{

/** \addtogroup grp_atomspace
 *  @{
 */

/**
 * TcpSocketNodes provide a stream of StringValues read from a TCP
 * socket, and written to it. This allows interactive text I/O
 * with an external process over a network (or localhost), e.g. a
 * human user running `socat` to connect to the socket.
 *
 * The node acts as a server: on open, it creates, binds, and listens
 * on the socket. The accept is deferred to the first read or write,
 * which blocks until a client connects. Once connected, line-oriented
 * text can be read and written.
 *
 * The close/read interaction is thread safe: the only way to break
 * out of a blocking read in one thread is to call close() from a
 * different thread.
 *
 * URI format: tcp://host:port (e.g. tcp://0.0.0.0:5000)
 *
 * This is experimental.
 * Unsolved issues:
 * -- Fails to handle lines longer than 4096
 * -- Accepts only one client at a time (this listen socket will
 *    accept only one connection at a time) This seems like a
 *    reasonable limitation at this time.
 */
class TcpSocketNode
	: public TextStreamNode
{
protected:
	mutable std::mutex _mtx;  // Protects _client_fd and coordinates close/read
	mutable int _client_fd;   // Accepted client connection fd
	mutable int _listen_fd;   // Listening socket file descriptor
	int _port;                // TCP port number
	mutable std::string _read_buf; // Partial-line read buffer

	void do_accept(void) const;
	virtual void do_write(const std::string&);

	virtual void open(const ValuePtr&);
	virtual void close(const ValuePtr&);
	// virtual void write(const ValuePtr&); inherited from StreamNode
	virtual bool connected(void) const;
	virtual void barrier(AtomSpace* = nullptr);
	virtual std::string do_read(void) const;

public:
	TcpSocketNode(const std::string&&);
	TcpSocketNode(Type t, const std::string&&);
	virtual ~TcpSocketNode();

	static Handle factory(const Handle&);
};

NODE_PTR_DECL(TcpSocketNode)
#define createTcpSocketNode CREATE_DECL(TcpSocketNode)

/** @}*/
} // namespace opencog

extern "C" {
void opencog_sensory_tcpsock_init(void);
};

#endif // _OPENCOG_TCP_SOCKET_NODE_H
