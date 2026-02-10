/*
 * opencog/atoms/ollama/OllamaNode.cc
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

#include <algorithm>
#include <httplib.h>

#include <opencog/util/exceptions.h>
#include <opencog/util/oc_assert.h>
#include <opencog/atoms/base/Node.h>
#include <opencog/atoms/value/BoolValue.h>
#include <opencog/atoms/value/LinkValue.h>
#include <opencog/atoms/value/StringValue.h>
#include <opencog/atoms/value/VoidValue.h>

#include <opencog/sensory/types/atom_types.h>
#include "OllamaNode.h"

using namespace opencog;

// ====================================================================

OllamaNode::OllamaNode(Type t, const std::string&& str) :
	TextStreamNode(t, std::move(str)),
	_loop(nullptr),
	_port(11434)
{
	OC_ASSERT(nameserver().isA(_type, OLLAMA_NODE),
		"Bad OllamaNode constructor!");
}

OllamaNode::OllamaNode(const std::string&& str) :
	TextStreamNode(OLLAMA_NODE, std::move(str)),
	_loop(nullptr),
	_port(11434)
{
}

OllamaNode::~OllamaNode()
{
	close(nullptr);
}

// ====================================================================
// URL format: ollama://host:port/model
// Examples:
//   ollama://localhost:11434/qwen3:8b
//   ollama://localhost/qwen3:8b      (port defaults to 11434)
//   ollama:///qwen3:8b               (host defaults to localhost)
//   qwen3:8b                         (bare model name, all defaults)
//
// XXX FIXME: instead of parsing the URL, all of the above should be
// configurable with calls to OllamaNode::setValue() before the open.
// But for now, using URLs is okay, I guess. Maybe we should have a
// generic URL-decoder node? Because this is a recurring pattern, e.g
// in RocksStorageNode and CogStorageNode ... which also should not use
// URL's, but should be configurable...
//
void OllamaNode::open(const ValuePtr& vurl)
{
	if (not vurl->is_type(STRING_VALUE) and
	    not vurl->is_type(NODE))
		throw RuntimeException(TRACE_INFO,
			"Expecting StringValue or Node; got \"%s\"\n",
			vurl->to_string().c_str());

	std::string uri;
	if (vurl->is_type(STRING_VALUE))
		uri = StringValueCast(vurl)->value()[0];
	if (vurl->is_type(NODE))
		uri = NodeCast(HandleCast(vurl))->get_name();

	_host = "localhost";
	_port = 11434;
	_model = "";
	_cancel = false;

	// Parse ollama:// URL
	if (0 == uri.compare(0, 9, "ollama://"))
	{
		// Skip "ollama://"
		size_t base = 9;
		size_t sls = uri.find('/', base);

		// Parse host:port portion
		std::string hostport;
		if (std::string::npos == sls)
		{
			// No slash after host — entire remainder is host:port
			// This means no model was given, which is an error.
			hostport = uri.substr(base);
			throw RuntimeException(TRACE_INFO,
				"No model specified in URL \"%s\"\n", uri.c_str());
		}
		else
		{
			hostport = uri.substr(base, sls - base);
			_model = uri.substr(sls + 1);
		}

		// Parse host and port from hostport string
		if (not hostport.empty())
		{
			size_t col = hostport.find(':');
			if (std::string::npos == col)
			{
				_host = hostport;
			}
			else
			{
				_host = hostport.substr(0, col);
				_port = atoi(hostport.substr(col + 1).c_str());
			}
		}

		// Empty host means localhost
		if (_host.empty()) _host = "localhost";
	}
	else
	{
		// Bare model name, no URL prefix.
		_model = uri;
	}

	if (_model.empty())
		throw RuntimeException(TRACE_INFO,
			"No model specified in \"%s\"\n", uri.c_str());

	printf("OllamaNode: connecting to %s:%d model=%s\n",
		_host.c_str(), _port, _model.c_str());

	// Verify that Ollama is reachable.
	httplib::Client cli(_host, _port);
	cli.set_connection_timeout(5);
	cli.set_read_timeout(5);
	auto res = cli.Get("/api/tags");
	if (not res)
		throw RuntimeException(TRACE_INFO,
			"Cannot reach Ollama at %s:%d -- is it running?\n",
			_host.c_str(), _port);

	if (200 != res->status)
		throw RuntimeException(TRACE_INFO,
			"Ollama at %s:%d returned status %d\n",
			_host.c_str(), _port, res->status);

	_qvp = createQueueValue();
	_req_queue = createQueueValue();

	// Start background thread to process requests.
	_loop = new std::thread(&OllamaNode::looper, this);
}

void OllamaNode::close(const ValuePtr&)
{
	if (nullptr == _loop) return;
	_cancel = true;

	// Close the request queue to unblock the looper.
	_req_queue->close();

	_loop->join();
	delete _loop;
	_loop = nullptr;

	_qvp = nullptr;
	_req_queue = nullptr;
}

bool OllamaNode::connected(void) const
{
	return (nullptr != _loop);
}

// ====================================================================
// JSON utility functions.
// These are simple and specific to the Ollama API formats.
// No need for a general-purpose JSON library.

// Escape a string for JSON embedding.
static std::string json_escape(const std::string& s)
{
	std::string result;
	result.reserve(s.size() + 16);
	for (char c : s)
	{
		switch (c)
		{
			case '"':  result += "\\\""; break;
			case '\\': result += "\\\\"; break;
			case '\n': result += "\\n"; break;
			case '\r': result += "\\r"; break;
			case '\t': result += "\\t"; break;
			default:   result += c; break;
		}
	}
	return result;
}

// Extract a string value for a given key from a JSON object string.
// Looks for "key":"value" and returns value (with JSON escapes decoded).
// Returns empty string if key not found.
static std::string json_get_string(const std::string& json,
                                   const std::string& key)
{
	std::string needle = "\"" + key + "\"";
	size_t pos = json.find(needle);
	if (std::string::npos == pos) return "";

	// Skip past the key and colon
	pos += needle.size();
	while (pos < json.size() and (json[pos] == ' ' or json[pos] == ':'))
		pos++;

	if (pos >= json.size() or json[pos] != '"') return "";
	pos++; // skip opening quote

	std::string result;
	while (pos < json.size() and json[pos] != '"')
	{
		if (json[pos] == '\\' and pos + 1 < json.size())
		{
			pos++;
			switch (json[pos])
			{
				case '"':  result += '"'; break;
				case '\\': result += '\\'; break;
				case 'n':  result += '\n'; break;
				case 'r':  result += '\r'; break;
				case 't':  result += '\t'; break;
				default:   result += json[pos]; break;
			}
		}
		else
		{
			result += json[pos];
		}
		pos++;
	}
	return result;
}

// ====================================================================
// HTTP request methods. These run in the looper thread.

std::string OllamaNode::do_generate(const std::string& prompt)
{
	std::string body =
		"{\"model\":\"" + json_escape(_model) + "\","
		"\"prompt\":\"" + json_escape(prompt) + "\","
		"\"stream\":false}";

	httplib::Client cli(_host, _port);
	cli.set_read_timeout(300); // LLM inference can be slow
	cli.set_write_timeout(10);

	auto res = cli.Post("/api/generate", body, "application/json");
	if (not res)
		return "[OllamaNode error: no response from server]";

	if (200 != res->status)
		return "[OllamaNode error: HTTP " + std::to_string(res->status) + "]";

	return json_get_string(res->body, "response");
}

std::string OllamaNode::do_chat(const std::string& json_messages)
{
	std::string body =
		"{\"model\":\"" + json_escape(_model) + "\","
		"\"messages\":" + json_messages + ","
		"\"stream\":false}";

	httplib::Client cli(_host, _port);
	cli.set_read_timeout(300);
	cli.set_write_timeout(10);

	auto res = cli.Post("/api/chat", body, "application/json");
	if (not res)
		return "[OllamaNode error: no response from server]";

	if (200 != res->status)
		return "[OllamaNode error: HTTP " + std::to_string(res->status) + "]";

	// The chat response has {"message":{"role":"assistant","content":"..."}}
	// We need to find "content" inside the "message" object.
	return json_get_string(res->body, "content");
}

// ====================================================================
// Background thread. Pulls requests from _req_queue, sends to Ollama,
// pushes responses onto _qvp.

void OllamaNode::looper(void)
{
	while (not _cancel)
	{
		ValuePtr req;
		try
		{
			req = _req_queue->remove();
		}
		catch (typename concurrent_queue<ValuePtr>::Canceled& e)
		{
			break;
		}

		if (_cancel) break;
		if (nullptr == req) continue;

		// A StringValue with one element means /api/generate.
		// A StringValue with two elements means /api/chat
		//   where element[0] is "chat" and element[1] is the JSON messages.
		if (req->is_type(STRING_VALUE))
		{
			StringValuePtr svp(StringValueCast(req));
			const std::vector<std::string>& strs = svp->value();

			std::string response;
			if (strs.size() >= 2 and strs[0] == "chat")
				response = do_chat(strs[1]);
			else if (not strs.empty())
				response = do_generate(strs[0]);
			else
				continue;

			// Replace newlines with spaces. LLM responses are
			// typically multi-line, but most consumers (e.g. IRC)
			// expect single-line strings.
			std::replace(response.begin(), response.end(), '\n', ' ');
			std::replace(response.begin(), response.end(), '\r', ' ');

			if (not _cancel and _qvp)
				_qvp->add(createStringValue(std::move(response)));
		}
	}
}

// ====================================================================
// Write methods. These push requests onto the request queue.

// write_one handles different Atomese value types.
void OllamaNode::write_one(const ValuePtr& command_data)
{
	if (nullptr == _loop)
		throw RuntimeException(TRACE_INFO,
			"OllamaNode not connected; call open first.\n");

	// Simple string: use /api/generate
	if (command_data->is_type(STRING_VALUE))
	{
		StringValuePtr svp(StringValueCast(command_data));
		const std::vector<std::string>& strs = svp->value();
		for (const std::string& str : strs)
		{
			ValuePtr req = createStringValue(str);
			_req_queue->add(std::move(req));
		}
		return;
	}

	// A single Node: use its name as the prompt, /api/generate.
	if (command_data->is_type(NODE))
	{
		std::string prompt = HandleCast(command_data)->get_name();
		ValuePtr req = createStringValue(std::move(prompt));
		_req_queue->add(std::move(req));
		return;
	}

	// LinkValue: interpret as chat messages.
	// Each element should be a LinkValue of two StringValues: [role, content]
	// Or it can be a LinkValue of StringValues with pairs.
	if (command_data->is_type(LINK_VALUE))
	{
		LinkValuePtr lvp(LinkValueCast(command_data));
		const ValueSeq& msgs = lvp->value();

		// Build the JSON messages array
		std::string json = "[";
		bool first = true;
		for (const ValuePtr& msg : msgs)
		{
			if (not msg->is_type(LINK_VALUE)) continue;
			LinkValuePtr pair(LinkValueCast(msg));
			if (pair->size() < 2) continue;

			std::string role, content;
			const ValuePtr& rv = pair->value()[0];
			const ValuePtr& cv = pair->value()[1];

			if (rv->is_type(STRING_VALUE))
				role = StringValueCast(rv)->value()[0];
			else if (rv->is_node())
				role = HandleCast(rv)->get_name();

			if (cv->is_type(STRING_VALUE))
				content = StringValueCast(cv)->value()[0];
			else if (cv->is_node())
				content = HandleCast(cv)->get_name();

			if (role.empty() or content.empty()) continue;

			if (not first) json += ",";
			json += "{\"role\":\"" + json_escape(role) +
			        "\",\"content\":\"" + json_escape(content) + "\"}";
			first = false;
		}
		json += "]";

		// Tag this as a chat request.
		ValuePtr req = createStringValue(
			std::vector<std::string>({"chat", json}));
		_req_queue->add(std::move(req));
		return;
	}

	// ListLink: interpret like LinkValue.
	if (LIST_LINK == command_data->get_type())
	{
		const HandleSeq& oset = HandleCast(command_data)->getOutgoingSet();

		// If it's a simple list of nodes, concatenate as prompt.
		bool all_nodes = true;
		for (const Handle& h : oset)
			if (not h->is_node()) { all_nodes = false; break; }

		if (all_nodes)
		{
			std::string prompt;
			for (const Handle& h : oset)
			{
				if (not prompt.empty()) prompt += " ";
				prompt += h->get_name();
			}
			ValuePtr req = createStringValue(std::move(prompt));
			_req_queue->add(std::move(req));
			return;
		}
	}

	throw RuntimeException(TRACE_INFO,
		"OllamaNode: unsupported data %s\n",
		command_data->to_string().c_str());
}

// do_write is called by TextStreamNode::do_write(ValuePtr) for plain strings.
void OllamaNode::do_write(const std::string& prompt)
{
	if (nullptr == _loop)
		throw RuntimeException(TRACE_INFO,
			"OllamaNode not connected; call open first.\n");

	ValuePtr req = createStringValue(prompt);
	_req_queue->add(std::move(req));
}

// ====================================================================
// Read methods.

ValuePtr OllamaNode::read(void) const
{
	if (nullptr == _loop) return createVoidValue();

	if (_qvp->is_closed() and 0 == _qvp->size())
		return createVoidValue();

	try
	{
		return _qvp->remove();
	}
	catch (typename concurrent_queue<ValuePtr>::Canceled& e)
	{}

	return createVoidValue();
}

ValuePtr OllamaNode::stream(void) const
{
	if (nullptr == _loop) return createVoidValue();

	return _qvp;
}

// ====================================================================

DEFINE_NODE_FACTORY(OllamaNode, OLLAMA_NODE);

void opencog_sensory_ollama_init(void)
{
	// Force shared lib ctors to run
};
