/*
 * opencog/atoms/sensory/TextFileNode.h
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

#ifndef _OPENCOG_TEXT_FILE_NODE_H
#define _OPENCOG_TEXT_FILE_NODE_H

#include <opencog/atoms/sensory/SensoryNode.h>

namespace opencog
{
/** \addtogroup grp_atomspace
 *  @{
 *
 * TextFileNode class. Basic File I/O.
 */
class TextFileNode : public SensoryNode
{
protected:
	bool _is_open;
	void init(void);

public:
	// Please to NOT use this constructor!
	TextFileNode(Type, const std::string&&);

public:
	TextFileNode(const std::string&&);

	TextFileNode(TextFileNode&) = delete;
	TextFileNode& operator=(const TextFileNode&) = delete;

	virtual bool is_executable() const { return true; }
	virtual ValuePtr execute(AtomSpace*, bool);

	static Handle factory(const Handle&);
};

NODE_PTR_DECL(TextFileNode)
#define createTextFileNode CREATE_DECL(TextFileNode)

/** @}*/
}

#endif // _OPENCOG_TEXT_FILE_NODE_H
