/*
 * opencog/atoms/sensory/TextFileNode.cc
 *
 * Copyright (C) 2024 Linas Vepstas
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

#include <opencog/util/oc_assert.h>
#include "TextFileNode.h"
#include "PhraseStream.h"

using namespace opencog;

// ============================================================
// Constructors

void TextFileNode::init(void)
{
	_is_open = false;
	// TODO: check if the URL type is supported.
}

TextFileNode::TextFileNode(Type t, const std::string&& s)
   : SensoryNode(t, std::move(s))
{
   OC_ASSERT(nameserver().isA(_type, TEXT_FILE_NODE),
      "Bad TextFileNode constructor!");
	init();
}

TextFileNode::TextFileNode(const std::string&& s)
   : SensoryNode(TEXT_FILE_NODE, std::move(s))
{
	init();
}

void TextFileNode::open(const std::string& opts)
{
	printf("yo file open\n");
	_is_open = true;
}

void TextFileNode::close(void)
{
	printf("yo file close\n");
	_is_open = false;
}

bool TextFileNode::connected(void)
{
	printf("yo file connect query\n");
	return _is_open;
}

// ============================================================

ValuePtr TextFileNode::execute(AtomSpace* as, bool silent)
{
	// Pass the URL to the stream; the stream will open it.
	return createPhraseStream(_name);
}

DEFINE_NODE_FACTORY(TextFileNode, TEXT_FILE_NODE)
