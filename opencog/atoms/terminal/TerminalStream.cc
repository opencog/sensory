/*
 * opencog/atoms/sensory/TerminalStream.cc
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

#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h> // for strerror()
#include <unistd.h>

#include <opencog/util/exceptions.h>
#include <opencog/util/oc_assert.h>
#include <opencog/atoms/base/Node.h>
#include <opencog/atoms/value/StringValue.h>
#include <opencog/atoms/value/ValueFactory.h>

#include <opencog/atoms/sensory-types/sensory_types.h>
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
	if (_fh)
		fclose (_fh);

	// Runs only if GC runs. This is a problem.
	kill (_xterm, SIGKILL);
}

void TerminalStream::init(void)
{
	_fh = nullptr;

	int fd = posix_openpt(O_RDWR|O_NOCTTY);
	if (0 > fd)
		throw RuntimeException(TRACE_INFO, "Can't open PTY %d %s",
			errno, strerror(errno));

	int rc = unlockpt(fd);
	if (0 != rc)
		throw RuntimeException(TRACE_INFO, "Can't unlock PTY %d %s",
			errno, strerror(errno));

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
	_xterm = fork();
	if (-1 == _xterm)
		throw RuntimeException(TRACE_INFO, "Failed to fork %d %s",
			errno, strerror(errno));

	if (0 == _xterm)
		execl("/usr/bin/xterm", "xterm", ccn.c_str(), (char *) NULL);

	printf("Created xterm pid=%d\n", _xterm);

	// Hmm. Seems like the right thing to do is to close the controlling
	// terminal created by open_pt() above, and open another, as a slave.
	// And I guess this works because fd was opened with O_NOCTTY
	close(fd);

	_fh = fopen(my_ptsname, "a+");

	fprintf(_fh, "Hello world this is so chill\n");
	fprintf(_fh, "Chillin\n");

	#define PTSZ 256
	char buff[PTSZ];
	for (int i=0; i<2500000; i++)
	{
		buff[0] = 0;
		char * s = fgets(buff, PTSZ, _fh);
		if (nullptr == s) break;
		printf("you %d said %s\n", i, buff);
		fprintf(_fh, "you %d said %s\n", i, buff);
	}
	printf("exited loop\n");

#if 0
	if (0 != url.compare(0, 8, "file:///"))
		throw RuntimeException(TRACE_INFO,
			"Unsupported URL \"%s\"\n", url.c_str());

	// Make a copy, for debuggingg purposes.
	_uri = url;

	// Ignore the first 7 chars "file://"
	const char* fpath = url.substr(7).c_str();
	_fh = fopen(fpath, "a+");

	if (nullptr == _fh)
	{
		int norr = errno;
		char buff[80];
		buff[0] = 0;
		// Apparently, we are getting the Gnu version of strerror_r
		// and not the XSI version. I suppose it doesn't matter.
		char * ers = strerror_r(norr, buff, 80);
		throw RuntimeException(TRACE_INFO,
			"Unable to open URL \"%s\"\nError was \"%s\"\n",
			url.c_str(), ers);
	}
#endif
}

// ==============================================================

ValuePtr TerminalStream::describe(AtomSpace* as, bool silent)
{
	throw RuntimeException(TRACE_INFO, "Not implemeneted");
	return Handle::UNDEFINED;
}

// ==============================================================

// This will read one line from the file stream, and return that line.
// So, a line-oriented, buffered interface. For now.
void TerminalStream::update() const
{
	if (nullptr == _fh) { _value.clear(); return; }

#define BUFSZ 4080
	char buff[BUFSZ];
	char* rd = fgets(buff, BUFSZ, _fh);
	if (nullptr == rd)
	{
		fclose(_fh);
		_fh = nullptr;
		_value.clear();
		return;
	}

	_value.resize(1);
	_value[0] = createNode(ITEM_NODE, buff);
}

// ==============================================================
// Write stuff to a file.

void TerminalStream::do_write(const std::string& str)
{
	fprintf(_fh, "%s", str.c_str());
}

// Write stuff to a file.
ValuePtr TerminalStream::write_out(AtomSpace* as, bool silent,
                                   const Handle& cref)
{
	if (nullptr == _fh)
		throw RuntimeException(TRACE_INFO,
			"Text stream not open\n");

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
