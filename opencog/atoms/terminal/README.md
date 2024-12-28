PTY Terminal I/O
================
Interacting with a Unix pty terminal. So, like IRC, but a lot simpler.

Examples
--------
Two. The `xterm-io.scm` shows basic usage, while `xterm-bridge.scm`
shows two of them, each echoing the other. Both examples are in the
[examples](../../../examples) directory.

Design
------
See the [Design Notes Part C](../../../DesignNotes-C.md) for a general
motivational discussion on why this class is needed.

The actual design is kind of bizarre. What I'd originally envisioned
was a C++ class that would open `/dev/pts/NN` and that there'd be some
Unix terminal that I could point at this PTY and use it to send &
receive text.

There is no such tool.  One could write one, by grabbing a copy of
the prehistoric `talk` command and hacking it. But this quickly devolves
into hacking `inetd.conf` and yuck. Do not want to go down that path: a
lot of work for little gain.

A different design alternative is to build a telnet server, and tell
everyone to telnet into it. This is a classic design point, I've done it
too many times in my life so, and its not that interesting, and is not
needed, its overkill for the intended research. Maybe some other day.

The final design is brute-force stupid but ugly: open a slave xterm.
You read that right. A skanky old xterm. It works, its super-simple
to code up, and is more than enough to solve all the other research
goals in this project. And so there it is. Brutish but servicable.

-----------------------------------
