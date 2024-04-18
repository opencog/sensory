/*
 * opencog/atoms/sensory/FileNode.h
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

#ifndef _OPENCOG_FILE_NODE_H
#define _OPENCOG_FILE_NODE_H

#include <opencog/atoms/sensory/SensoryNode.h>

namespace opencog
{
/** \addtogroup grp_atomspace
 *  @{
 *
 * FileNode class. Basic File I/O.
 */
class FileNode : public SensoryNode
{
protected:
	bool _is_open;
	void init(void);

	virtual void open(const std::string& flags);
	virtual void close(void);
	virtual bool connected(void);

public:
	// Please to NOT use this constructor!
	FileNode(Type, const std::string&&);

public:
	FileNode(const std::string&&);

	FileNode(FileNode&) = delete;
	FileNode& operator=(const FileNode&) = delete;

	virtual bool is_executable() const { return true; }
	virtual ValuePtr execute(AtomSpace*, bool);

	static Handle factory(const Handle&);
};

NODE_PTR_DECL(FileNode)
#define createFileNode CREATE_DECL(FileNode)

/** @}*/
}

#endif // _OPENCOG_FILE_NODE_H
