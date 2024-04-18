/*
 * opencog/atoms/sensory/FileNode.cc
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
#include <opencog/atoms/value/LinkStreamValue.h>
#include "FileNode.h"
#include "PhraseStream.h"

using namespace opencog;

// ============================================================
// Constructors

void FileNode::init(void)
{
	// TODO: check if the URL type is supported.
}

FileNode::FileNode(Type t, const std::string&& s)
   : Node(t, std::move(s))
{
   OC_ASSERT(nameserver().isA(_type, FILE_NODE),
      "Bad FileNode constructor!");
	init();
}

FileNode::FileNode(const std::string&& s)
   : Node(FILE_NODE, std::move(s))
{
	init();
}

// ============================================================

ValuePtr FileNode::execute(AtomSpace* as, bool silent)
{
	// Pass the URL to the stream; the stream will open it.
	return createPhraseStream(_name);
}

DEFINE_NODE_FACTORY(FileNode, FILE_NODE)
