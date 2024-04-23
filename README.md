Sensory Atomese
===============
This repo explores how perception and action with an external world
might work with the [AtomSpace](https://github.com/opencog/atomspace).

TL;DR: Implementation for low-level AtomSpace sensory I/O Atoms.
Provides a basic file I/O stream, and an IRC chat stream.

Philosophical Overview
----------------------
The issue for any agent is being able to perceive the environment that
it is in, and then being able to interact with this environment.

For OpenCog, and, specifically, for OpenCog Atomese, all interaction,
knowledge, data and reasoning is represented with and performed by
Atoms (stored in a hypergraph database) and Values (transient data
flowing through a network defined by Atoms).

It is not hard to generate Atoms, flow some Values around, and perform
some action (control a robot, say some words). The hard part is to
(conceptually, philosophically) understand how an agent can represent
the external world (with Atoms and Values), and how it should go about
doing things. The agent needs to perceive the environment in some way
that results in an AtomSpace representation that is easy for agent
subsystems to work with. This perception must include the agents
awareness of the different kinds of actions it might be able to perform
upon interacting with the external world.  That is, before one can
attempt to model the external world, one must be able to model ones own
ability to perform action. This is where the boundary between the agent
and the external world lies: the boundary of the controllable.

Traditional human conceptions of senses include sight and hearing;
traditional ideas consist of moving (robot) limbs. See, for example,
the git repo [opencog/vision](https://github.com/opencog/vision)
for OpenCV-based vision Atomese. (Note: It is at version 0.0.2)

The task being tackled here is at once much simpler and much harder:
exploring the unix filesystem, and interacting via chat. This might
sound easy, trivially easy, even, if you're a software developer.
The hard part is this: how does an agent know what a "file" is?
What a "directory" is? Actions it can perform are to walk the directory
tree; but why? Is it somehow "fun" for the agent to walk directories
and look at files? What should it do next? Read the same file again,
or maybe try some other file? Will the agent notice that maybe some
file has changed? If it notices, what should it do? What does it mean,
personally, to the agent, that some file changed? Should it care? Should
it be curious?

The agent can create files. Does it notice that it has created them?
Does it recognize those files as works of it's own making? Should it
read them, and admire the contents? Or perform some processing on them?
Or is this like eating excrement? What part of the "external world"
(the filesystem) is perceived to be truly external, and what part is
"part of the agent itself"? What does it mean to exist and operate in
a world like this? What's the fundamental nature of action and
perception?

When an agent "looks at" a file, or "looks at" the list of users on
a chat channel, is this an action, or a perception? Both, of course:
the agent must make a conscious decision to look (take an action) and
then, upon taking that action, sense the results (get the text in the
file or the chat text). After this, it must "perceive" the results:
figure out what they "mean".

These are the questions that seem to matter, for agent design. The code
in this git repo is some extremely low-level, crude Atomese interfaces
that try to expose these issues up into the AtomSpace.

Currently, two interfaces are bing explored: a unix filesystem
interface, and an IRC chat interface. Hopefully, this is broad enough to
expose some of the design issues. Basically, chat is not like a
filesystem: there are public channels, there are private conversations.
The kind of sensory information coming from chat is just different than
the sensory information comeing from files (even though, as a software
engineer, one could map chat I/O to a filesystem-style interface.)
Trying to support both keeps us honest.

### Status
Version 0.2.2 -- Experimental. Basic demos actually work. Overall
architecture is ok-ish. The grand questions above remain mysterious.

Provides:
* Basic File I/O stream.
* Prototype IRC chatbot stream.

See the [examples](examples) directory.

### Design Overview
The Atomese agent framework needs to have some way of interacting
with it's environment. Obviously, reading, writing, seeing, hearing.
More narrowly: the ability to read a text file in the local file system.
The ability to read directory contents, to move throuogh directories.
The ability to behave as a chatbot, e.g. on IRC, but also as a
javascript chatbot running in a web-page. Also possibly running
free on twitter, discord, youtube.

The goal here is to prove the very lowest layers, just the glue,
to convert that stuff into Atomese Atoms that higher-layer Atomese
agents make use of to communicate with, interact with the external
world.

A general overview can be found in the AGI 2022 paper:
[Purely Symbolic Induction of Structure](https://github.com/opencog/learn/tree/master/learn-lang-diary/agi-2022/grammar-induction.pdf).

General system architecture is discussed in a number of places,
including the various PDF and LyX files located at:
* [AtomSpace Sheaves & Graphs](https://github.com/opencog/atomspace/tree/master/opencog/sheaf)
* [OpenCog Learn Project](https://github.com/opencog/learn) and
  especially the "diary" subdirectory there.

See also:
* [Atomese Agents Project](https://github.com/opencog/agents). This is
  in the pre-prototype phase, but is the current focus of attention.

### Design specifics
Details of the design in this git repo are explored in several places:

* [DesignNotes](DesignNotes.md) -- Design of WriteLink
* [IRChatStream](opencog/atoms/irc/README.md) -- IRC chat design.

### Building
This git repo follows the same directory structure and coding
conventions used in other OpenCog/AtomSpace projects. This cannot be
compiled before installing the prerequisite
[AtomSpace](https://github.com/opencog/atomspace). So build and
install that first.

Then:
```
mkdir build; cd build; cmake ..
make -j
sudo make install
```

### Examples
See the [examples](examples) directory.
