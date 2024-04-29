File and Directory Navigation
=============================
Navigating through directories and interacting with files.

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

Additional ruminations and design criteria can be found in the
[IRChatStream README](../irc/README.md). The goal is to have both of
these streams be as similar as is reasonable, so that the agent does
not need to re-learn eintirely new interaction styles.

Examples
--------
The `TextFileStream` can be used to read and write files. See the
See the [examples](../../../examples) directory.

Design
------
OK, Let's take the notes in [IRChatStream README](../irc/README.md) to
heart. It says that actions, and the result of those actions, should be
described by a `ChoiceLink` of `Section`s. The current filesys example,
as it stands, treats this as a DTD and attempts to manually pick it
apart and create just one single link. And ... almost works but fails.
Because it's complicated.  This hooking-up of things needs to be
abstracted and automated.



-----------------------------------
