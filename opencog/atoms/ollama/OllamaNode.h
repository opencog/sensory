/*
 * opencog/atoms/ollama/OllamaNode.h
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

#ifndef _OPENCOG_OLLAMA_NODE_H
#define _OPENCOG_OLLAMA_NODE_H

#include <atomic>
#include <thread>
#include <opencog/atoms/value/QueueValue.h>
#include <opencog/atoms/sensory/TextStreamNode.h>

namespace opencog
{

/** \addtogroup grp_atomspace
 *  @{
 */

/**
 * OllamaNode provides a sensory interface to the Ollama LLM server.
 * It allows sending prompts and receiving responses via the standard
 * sensory open/read/write/close predicates.
 *
 * URL format: ollama://host:port/model
 * Example: ollama://localhost:11434/qwen3:8b
 *
 * Writing a plain string sends a /api/generate request.
 * Writing a LinkValue of role/content pairs sends a /api/chat request.
 *
 * XXX FIXME: See the README for criticism. This is a bit disappointing,
 * as this is just a cheap and cheesy FFI wrapper around Ollama, and
 * utterly fails to explore the nature of interfaces. And, as such,
 * it fails to conform to the project goals. But, for now, its usable.
 * It fulfils some basic needs. So Caveat Emptor: You can use this,
 * but its not really going in the correct long-term direction.
 */
class OllamaNode
	: public TextStreamNode
{
private:
	std::thread* _loop;
	std::atomic<bool> _cancel;

	// Ollama connection details, parsed from URL.
	std::string _host;
	int _port;
	std::string _model;

	// Request queue: write() pushes requests, looper() pops them.
	QueueValuePtr _req_queue;

	void looper(void);
	std::string do_generate(const std::string& prompt);
	std::string do_chat(const std::string& json_messages);

protected:
	QueueValuePtr _qvp;

	virtual void open(const ValuePtr&);
	virtual void close(const ValuePtr&);
	virtual void write_one(const ValuePtr&);
	virtual void do_write(const std::string&);
	virtual bool connected(void) const;
	virtual ValuePtr read(void) const;
	virtual ValuePtr stream(void) const;

public:
	OllamaNode(const std::string&&);
	OllamaNode(Type, const std::string&&);
	virtual ~OllamaNode();

	static Handle factory(const Handle&);
};

NODE_PTR_DECL(OllamaNode)
#define createOllamaNode CREATE_DECL(OllamaNode)

/** @}*/
} // namespace opencog

extern "C" {
void opencog_sensory_ollama_init(void);
};

#endif // _OPENCOG_OLLAMA_NODE_H
