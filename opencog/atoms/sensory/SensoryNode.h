/*
 * opencog/atoms/sensory/SensoryNode.h
 *
 * Copyright (C) 2015,2020,2022,2024 Linas Vepstas
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
#include <opencog/atoms/sensory-types/sensory_types.h>

namespace opencog
{
/** \addtogroup grp_atomspace
 *  @{
 */

// Similar to a StorageNode, but a simpler API that can be
// implemented in a more general setting.  API offers open, close
// read, write, erase functions. Some are virtual.
class SensoryNode : public Node
{
public:
	SensoryNode(Type, std::string);
	virtual ~SensoryNode();

	// ----------------------------------------------------------------
	// Operations regarding the connection to the remote URI.
	/**
	 * Open a connection to the indicated URI.
	 */
	virtual void open(const std::string& flags) = 0;

	/**
	 * Close an active connection.
	 */
	virtual void close(void) = 0;

	/**
	 * Return true if the connection to the remote end appears to
	 * be established and functioning.
	 */
	virtual bool connected(void) = 0;

	/**
	 * Initialize storage at the remote end. There must already be
	 * an open connection to the remote end; and the remote end must
	 * be vacant or empty.  For example: for an SQL server, this can
	 * be used to create the database, the tables in the database for
	 * the first time.
	 */
	virtual void create(void) = 0;

	/**
	 * Destroy the storage at the remote end. Empties the remote end of
	 * data, and then undoes whaterver `create()` did. Remote ends might
	 * not honor this request, e.g. if other clients have open
	 * connections.
	 */
	virtual void destroy(void) = 0;

	/**
	 * Erase the entire contents of the remote end. Performs a bulk
	 * deletion of all data.
	 */
	virtual void erase(void) = 0;

	/**
	 * Return debug diagnostics and/or performance monitoring stats.
	 */
	virtual std::string monitor(void);

	// ----------------------------------------------------------------
	// Operations regarding specific atomspace contents.

	/**
	 * Make sure all atom writes have completed, before returning.
	 * This only has an effect when the atomspace is backed by some
	 * sort of storage, or is sending atoms to some remote location
	 * asynchronously. This simply guarantees that the asynch
	 * operations have completed.
	 * NB: at this time, we don't distinguish barrier and flush.
	 */
	void barrier(AtomSpace* = nullptr);
};

NODE_PTR_DECL(SensoryNode)

/** @}*/
} // namespace opencog

extern "C" {
void opencog_sensory_init(void);
};

#endif // _OPENCOG_SENSORY_NODE_H
