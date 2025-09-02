/*
 * opencog/atoms/sensory/FileSysNode.h
 *
 * Copyright (C) 2024,2025 Linas Vepstas
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

#ifndef _OPENCOG_FILE_SYS_NODE_H
#define _OPENCOG_FILE_SYS_NODE_H

#include <opencog/atoms/sensory/TextStreamNode.h>

namespace opencog
{

/** \addtogroup grp_atomspace
 *  @{
 */

/**
 * FileSysNode provides an object capable of navigating a filesystem.
 * This is experimental.
 */
class FileSysNode
	: public TextStreamNode
{
protected:
	void init(const std::string&);
	mutable std::string _cwd;

	virtual bool connected(void) const;
	virtual void close(const ValuePtr&);
	virtual void do_write(const std::string&);

	virtual ValuePtr write_out(AtomSpace*, bool, const Handle&);

public:
	FileSysNode(const std::string&&);
	FileSysNode(Type, const std::string&&);
	virtual ~FileSysNode();

	static Handle factory(const Handle&);
};

NODE_PTR_DECL(FileSysNode)
#define createFileSysNode CREATE_DECL(FileSysNode)

/** @}*/
} // namespace opencog

#endif // _OPENCOG_FILE_SYS_NODE_H
