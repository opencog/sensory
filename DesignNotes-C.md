Design Notes Part C -- Filesystem
=================================
***Late April 2024***

File and Directory Navigation - Navigating through directories and
interacting with files. Exploration of using a Link Grammar formalism
for hooking things up.

Perception and Action
---------------------
How does an agent interact with a file system? How can a file system be
exposed, in Atomese, such that an agent can efficiently, easily and
effectively interact with that file system?

This is a beguilingly simple question: files and directories are a part
of software development basics. It becomes harder if you imagine
yourself (or a robot) locked into a virtual universe where files and
directories are all that there are. How do you walk about? Where is your
current position? How do you decide what to do next? Should you read a
file, or write it? Should you read it again and again? Perhaps you
should avoid places that you've visited before. Is there an explore vs.
exploit task here, where one could "exploit" interesting files, vs.
"explore" to find other interesting things?

Wrapped up in the above are low-level and high-level questions. At the
lowest level, we simply want to create an API that an agent can use to
explore the file system. That'ts what the code here is for.

Ruminations and design criteria are given in
[Design Notes Part B -- IRC](DesignNotes-B.md). The goal is to have
both the IRC and the File System streams as similar as is reasonable,
so that the agent does not need to re-learn entirely new interaction
styles.

Examples
--------
The `TextFileStream` can be used to read and write files. See the
See the [examples](../../../examples) directory.

Link Grammar
------------
OK, Let's take the notes in [Design Notes B](DesignNotes-B.md) to
heart. It says that actions, and the result of those actions, should be
described by a `ChoiceLink` of `Section`s. The
[current filesys example](examples/filesys.scm),
as it stands, treats this as a DTD and attempts to manually pick it
apart and create just one single link. And ... almost works but fails.
Because it's complicated.  This hooking-up of things needs to be
abstracted and automated. How?

Part of the problem is that theres a DTD for the filesys commands,
but no matching connector-sets describing the pipeline. So we really
want:
* An API for terminal I/O
* An LG-style DTD for the terminal I/O
* A linker that can construct a pipeline from the Filesys 'ls' command
  to the terminal. The linker auto-connects, so that none of the icky
  code in the demo would need to be written.
* Repeat above, for IRC. Terminal I/O is easier, cause we can punt on
  channel joins, for now. Alternately, could build a pipe from 'ls' to
  a file, but that is lesss fun, because files are not interactive.

WTF is this? The 1960's? I'm reinventing terminal I/O? Really? Didn't
someone else do this, 60 years ago? Am I crazy?

Another task is to pipe the output of the IRC channel list command to a
file (or a terminal), or to create a spybot, that reads IRC channel
convos and writes them to a file.

Can this be done with a single linkage, or are multiple linkages needed?
The IRC echobot seems to require multiple linkages. It reads, then
applies some decision filters to figure out if it is on a public or
private channel, then it applies one of two different kinds of
responses, depending on which, and finally, it writes. So a grand total
of six elements and two linkages:

* Read target
* Public/private decision filter
* Two different reply formulators
* If on public, then only when addressed filter
* Write target

Try it in ASCII graphics
```
                         x --> pub-maybe --> pub-reply -x
                        /                                \
   Read --> pub/priv --x                                  x--> Write
                        \                                /
                         x ----> private reply >--------x
```
Each of the dashed lines is an LG link; the link-type is NOT noted.
The data flow is mostly text. However, the data to indicate if a
messages came via public or private channel, how does that work?
Does it come as a distinct data type flowing on a pipeline? Or
does it manifest into a different connector?  Maybe both, so that
there is a multiplexor or router node, that routes to different pipes,
depending on the prior message it received?

Am I navel gazing?
------------------
The above is so low level, it feels crazy. Lets take a more careful
look, and see if that's true, if there's a better design or a simpler
design or the whole thing is pointless. So, bullet by bullet.

* Terminal I/O
* Message routing/multiplexing


-----------------------------------
