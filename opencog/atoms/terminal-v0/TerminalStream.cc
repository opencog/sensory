/*
 * opencog/atoms/sensory-v0/TerminalStream.cc
 *
 * Copyright (C) 2020 Linas Vepstas
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
#include <opencog/atoms/value/ValueFactory.h>

#include <opencog/sensory-v0/types/atom_types.h>
#include "TerminalStream.h"

using namespace opencog;

// Terminal I/O using posix_openpt(), ptsname(), grantpt(), and unlockpt()
// ttyname() pts(4), pty(7)

TerminalStream::TerminalStream(Type t, const std::string& str)
	: OutputStream(t)
{
	OC_ASSERT(nameserver().isA(_type, TERMINAL_STREAM),
		"Bad TerminalStream constructor!");
	init();
}

TerminalStream::TerminalStream(void)
	: OutputStream(TERMINAL_STREAM)
{
	init();
}

TerminalStream::TerminalStream(const ValueSeq& seq)
	: OutputStream(TERMINAL_STREAM) // seq
{
	init();
}

TerminalStream::~TerminalStream()
{
	// Runs only if GC runs. This is a problem.
	halt();
}

void TerminalStream::halt(void) const
{
	if (_fh)
		fclose (_fh);
	_fh = nullptr;

	if (_xterm_pid)
		kill (_xterm_pid, SIGKILL);
	_xterm_pid= 0;

	_value.clear();
}

void TerminalStream::init(void)
{
	do_describe();

	_fh = nullptr;

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
	close(fd);

	_fh = fopen(my_ptsname, "a+");
}

// ==============================================================

Handle _global_desc = Handle::UNDEFINED;

void TerminalStream::do_describe(void)
{
	if (_global_desc) return;

	HandleSeq cmds;

	// Describe exactly how to Open this stream.
	// It needs no special arguments.
	Handle open_cmd =
		make_description("the open terminal command",
		                 "OpenLink", "TerminalStream");
	cmds.emplace_back(open_cmd);

	// Write text
	Handle write_cmd =
		make_description("the write stuff command",
		                 "WriteLink", "ItemNode");
	cmds.emplace_back(write_cmd);

	_global_desc = createLink(cmds, CHOICE_LINK);
}

// This is totally bogus because it is unused.
// This should be class static member
ValuePtr TerminalStream::describe(AtomSpace* as, bool silent)
{
	if (_description) return as->add_atom(_description);
	_description = as->add_atom(_global_desc);
	return _description;
}

// ==============================================================

// This will read one line from the file stream, and return that line.
// So, a line-oriented, buffered interface. For now.
// This blocks, waiting for input, if there is no input.
void TerminalStream::update() const
{
	if (nullptr == _fh) { _value.clear(); return; }

#define BUFSZ 4080
	char buff[BUFSZ];
	buff[0] = 0;

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
		return;
	}

	_value.resize(1);
	_value[0] = createStringValue(buff);
}

// ==============================================================
// Write stuff to a file.

void TerminalStream::do_write(const std::string& str)
{
	if (nullptr == _fh)
		throw RuntimeException(TRACE_INFO,
			"Text stream not open\n");

	fprintf(_fh, "%s", str.c_str());
	fflush(_fh);
}

// Write stuff to a file.
ValuePtr TerminalStream::write_out(AtomSpace* as, bool silent,
                                   const Handle& cref)
{
	return do_write_out(as, silent, cref);
}

// ==============================================================

// Adds factory when library is loaded.
DEFINE_VALUE_FACTORY(TERMINAL_STREAM, createTerminalStream)
DEFINE_VALUE_FACTORY(TERMINAL_STREAM, createTerminalStream, ValueSeq)

// ====================================================================

void opencog_sensory_terminal_init(void)
{
   // Force shared lib ctors to run
};
