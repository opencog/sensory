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

#include <opencog/atoms/base/Node.h>
#include <opencog/sensory/types/atom_types.h>

namespace opencog
{
/** \addtogroup grp_atomspace
 *  @{
 */

// Dispatcher for setValue, getValue.
// Also, holder of URL's.
class SensoryNode : public Node
{
protected:
	SensoryNode(Type, const std::string&&);

	/**
	 * Return debug diagnostics and/or performance monitoring stats.
	 */
	virtual std::string monitor(void) const;

public:
	SensoryNode(const std::string&& uri)
		: SensoryNode(SENSORY_NODE, std::move(uri)) {}
	virtual ~SensoryNode();
};

NODE_PTR_DECL(SensoryNode)

/** @}*/
} // namespace opencog

extern "C" {
void opencog_sensory_init(void);
};

#endif // _OPENCOG_SENSORY_NODE_H
