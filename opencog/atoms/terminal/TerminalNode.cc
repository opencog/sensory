/*
 * opencog/atoms/sensory/TerminalNode.cc
 *
 * Copyright (C) 2024, 2025 Linas Vepstas
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

#include <sys/ioctl.h>
#include <asm/termbits.h>

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h> // for strerror()
#include <unistd.h>

#include <opencog/util/exceptions.h>
#include <opencog/util/oc_assert.h>
#include <opencog/atomspace/AtomSpace.h>
#include <opencog/atoms/base/Link.h>
#include <opencog/atoms/base/Node.h>
#include <opencog/atoms/value/StringValue.h>

#include <opencog/sensory/types/atom_types.h>
#include "TerminalNode.h"

using namespace opencog;

// Terminal I/O using posix_openpt(), ptsname(), grantpt(), and unlockpt()
// ttyname() pts(4), pty(7)

TerminalNode::TerminalNode(const std::string&& str) :
	TextStreamNode(TERMINAL_NODE, std::move(str)),
	_fh(nullptr),
	_xterm_pid(0)
{
}

TerminalNode::TerminalNode(Type t, const std::string&& str) :
	TextStreamNode(t, std::move(str)),
	_fh(nullptr),
	_xterm_pid(0)
{
	OC_ASSERT(nameserver().isA(_type, TERMINAL_NODE),
		"Bad TerminalNode constructor!");
}

TerminalNode::~TerminalNode()
{
	// Runs only if GC runs. This is a problem.
	halt();
}

void TerminalNode::halt(void) const
{
	if (_fh)
		fclose (_fh);
	_fh = nullptr;

	if (_xterm_pid)
		kill (_xterm_pid, SIGKILL);
	_xterm_pid= 0;
}

void TerminalNode::open(const ValuePtr& retype)
{
	TextStreamNode::open(retype);

	if (_fh) return;

	int fd = posix_openpt(O_RDWR|O_NOCTTY);
	if (0 > fd)
		throw RuntimeException(TRACE_INFO, "Can't open PTY %d %s",
			errno, strerror(errno));

	int rc = unlockpt(fd);
	if (0 != rc)
		throw RuntimeException(TRACE_INFO, "Can't unlock PTY %d %s",
			errno, strerror(errno));

	// Has no effect, but whatever.
	// int arg = TIOCPKT_FLUSHWRITE;
	// rc = ioctl(fd, TIOCPKT, &arg);

	// Get the PTY name
	#define PTSZ 256
	char my_ptsname[PTSZ];
	rc = ptsname_r(fd, my_ptsname, PTSZ);
	if (0 != rc)
		throw RuntimeException(TRACE_INFO, "Can't get PTY name %d %s",
			errno, strerror(errno));

	printf("Opened %s\n", my_ptsname);

	// Build arguments for xterm
	std::string ccn = "-S";
	ccn += my_ptsname;
	ccn += "/" + std::to_string(fd);

	// Insane old-school hackery
	_xterm_pid = fork();
	if (-1 == _xterm_pid)
		throw RuntimeException(TRACE_INFO, "Failed to fork: (%d) %s",
			errno, strerror(errno));

	if (0 == _xterm_pid)
	{
		execl("/usr/bin/xterm", "xterm", ccn.c_str(), (char *) NULL);
		fprintf(stderr, "%s: line %d\n", __FILE__, __LINE__);
		fprintf(stderr, "Failed to start /usr/bin/xterm: (%d) %s\n",
			errno, strerror(errno));
		fprintf(stderr, "Make sure that xterm is installed!\n");
		exit(1);
	}

	printf("Created xterm pid=%d\n", _xterm_pid);

	// Hmm. Seems like the right thing to do is to close the terminal
	// created by open_pt() above, and open another, as a slave.
	// And I guess this works because fd was opened with O_NOCTTY
	// The alternative is `_fh = fdopen(fd, "a+")` but this flakes.
	::close(fd);

	_fh = fopen(my_ptsname, "a+");
}

void TerminalNode::close(const ValuePtr& ignore)
{
	halt();
}

bool TerminalNode::connected(void) const
{
	return (nullptr != _fh);
}

// ==============================================================

// This will read one line from the file stream, and return that line.
// So, a line-oriented, buffered interface. For now.
// This blocks, waiting for input, if there is no input.
std::string TerminalNode::do_read(void) const
{
	static const std::string empty_string;

	if (nullptr == _fh) return empty_string;

#define BUFSZ 256
	std::string str(BUFSZ, 0);
	char* buff = str.data();

	// Locking and blocking. There seems to be a feature/bug in some
	// combinations of linux kernel + glibc + xterm that prevents the
	// `xterm-bridge.scm` demo from operating nicely, if plain old
	// fgets() is used. I tried everything, including ioctl(TIOCPKT).
	// The problem seems to be that fgets() grabs some lock, and so
	// when there are two threads using two pipes to talk to two xterm
	// processes, data is not sent/received in one pipe until the
	// other is nudged, momentarily dropping the lock.  You can see
	// the wonky behavior in the demo. Using fgets_unlocked() allows
	// the demo to run "normally", at the cost that sometimes, the
	// same data is returned twice! WTF. I can't tell if this is a
	// feature or a bug or what the heck this is supposed to be.
	// However, the xterm demo is low-priority, so further debugging
	// seems unjustified.
#ifdef _GNU_SOURCE
	char* rd = fgets_unlocked(buff, BUFSZ, _fh);
#else
	char* rd = fgets(buff, BUFSZ, _fh);
#endif

	// nullptr is EOF
	if (nullptr == rd)
	{
		halt();
		return empty_string;
	}

	return str;
}

// ==============================================================
// Write stuff to a file.

void TerminalNode::do_write(const std::string& str)
{
	if (nullptr == _fh)
		throw RuntimeException(TRACE_INFO,
			"Can't write to xterm `%s`: It's not open\n",
			this->to_string().c_str());

	fprintf(_fh, "%s", str.c_str());
	fflush(_fh);
}

// ==============================================================

// Adds factory when library is loaded.
DEFINE_NODE_FACTORY(TerminalNode, TERMINAL_NODE);

// ====================================================================

void opencog_sensory_terminal_init(void)
{
   // Force shared lib ctors to run
};
