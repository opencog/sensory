/*
 * opencog/atoms/sensory/SensoryNode.h
 *
 * Copyright (C) 2015,2020,2022,2024,2025 Linas Vepstas
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

#ifndef _OPENCOG_SENSORY_NODE_H
#define _OPENCOG_SENSORY_NODE_H

#include <opencog/atoms/core/ObjectNode.h>
#include <opencog/sensory/types/atom_types.h>

namespace opencog
{
/** \addtogroup grp_atomspace
 *  @{
 */

class SensoryNode;
extern template class ObjectCRTP<SensoryNode>;

// Dispatcher for setValue, getValue.
// Also, holder of URL's.
class SensoryNode : public ObjectCRTP<SensoryNode>
{
	friend class ReadStream;
	friend class StringStream;
	friend class ObjectCRTP<SensoryNode>;

protected:
	SensoryNode(Type, const std::string&&);

	static constexpr const char* _messages[] = {
		"*-open-*",
		"*-close-*",
		"*-write-*",
		"*-barrier-*",
		"*-follow-*",
		"*-connected?-*",
		"*-read-*",
		"*-stream-*",
		"*-monitor-*"
	};

	/**
	 * Default API that sensory nodes must provide. Similar to
	 * the current StorageNode API.
	 */
	virtual void open(const ValuePtr&) = 0;
	virtual void close(const ValuePtr&) = 0;
	virtual void write(const ValuePtr&) = 0;
	virtual void barrier(AtomSpace* = nullptr);
	virtual void follow(const ValuePtr&);

	virtual bool connected(void) const = 0;
	virtual ValuePtr read(void) const = 0;
	virtual ValuePtr stream(void) const = 0;

public:
	virtual ~SensoryNode();

	virtual void setValue(const Handle& key, const ValuePtr& value);
	virtual ValuePtr getValue(const Handle& key) const;
};

NODE_PTR_DECL(SensoryNode)

/** @}*/
} // namespace opencog

extern "C" {
void opencog_sensory_init(void);
};

#endif // _OPENCOG_SENSORY_NODE_H
