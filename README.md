Sensory Atomese
===============
This repo explores how perception and action within an external world
might work with the [AtomSpace](https://github.com/opencog/atomspace).

TL;DR: Explores philosophical approaches to perception & action via
actual, working code using low-level AtomSpace sensory I/O Atoms.
A software conception of "basal cognition".
The experimental lab for this is "perceiving" filesystem files,
"moving" through directories, and likewise for IRC chat streams.

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
subsystems to work with. This perception must include the agent's
awareness of the different kinds of actions it might be able to perform
upon interacting with the external world.  That is, before one can
attempt to model the external world, one must be able to model one's own
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

Currently, two interfaces are being explored: a unix filesystem
interface, and an IRC chat interface. Hopefully, this is broad enough to
expose some of the design issues. Basically, chat is not like a
filesystem: there is a large variety of IRC commands, there are public
channels, there are private conversations. They are bi-directional.
The kind of sensory information coming from chat is just different than
the sensory information coming from files (even though, as a clever
software engineer, one could map chat I/O to a filesystem-style
interface.) The point here is not to be "clever", but to design
action-perception correctly.  Trying to support very different kinds
of sensorimotor systems keeps us honest.

Typed Pipes and Data Processing Networks
----------------------------------------
In unix, there is the conception of a "pipe", having two endpoints. A
pair of unix processes can communicate "data" across a pipe, merely by
opening each endpoint, and reading/writing data to it. Internet sockets
are a special case of pipes, where the connected processes are running
on different computers somewhere on the internet.

Unix pipes are not typed: there is no a priori way of knowing what kind
of data might come flowing down them. Could be anything. For a pair of
processes to communicate, they must agree on the message set passing
through the pipe. The current solution to this is the IETF RFC's, which
are a rich collection of human-readable documents describing datastream
formats at the meta level. In a few rare cases, one can get a machine-
-readable description of the data. An example of this is the DTD, the
[Data Type Definition](https://en.wikipedia.org/wiki/Document_type_definition),
which is used by web browsers to figure out what kind of HTML is being
delivered (although the DTD is meant to be general enough for "any" use.)
Other examples include the X.500 and LDAP schemas, as well as SNMP.

However, there is no generic way of asking a pipe "hey mister pipe, what
are you? What kind of data passes over you?" or "how do I communicate
with whatever is at the other end of this pipe?" Usually, these
questions are resolved by some sort of hand-shaking and negotiation
when two parties connect.

The experiment being done here, in this git repo, in this code-base, is
to assign a type to a pipe. This replaces the earliest stages of
protocol negotiation: if a system wishes only connect to pipes of type
`FOO`, then it can know what is available a priori, by examining the
connection types attached to that pipe. If they are
`BAR+ or FOO+ or BLITZ+`, then we're good: the `or` is a disjunctive-or,
a menu choice of what is being served on that pipe. Upon opening that
pipe, some additional data descriptors might be served up, again in the
form of a menu choice. If the communicating processes wish to exchange
text data, when eventually find `TEXT-` and `TEXT+`, which are two
connectors stating "I'll send you text data" and "That's great, because
I can receive text data".

So far, so good. This is just plain-old ordinary computer science, so
far. The twist is that these data descriptors are being written as Link
Grammar (LG) connector types. Link Grammar is a language parser: given a
collection of "words", to which a collection of connectors are attached,
the parser can connect up the connectors to create "links". The linkages
are such that the endpoints always agree as to the type of the
connector.

The twist of using Link Grammar to create linkages changes the focus
from pair-wise, peer-to-peer connections, to a more global network
connection perspective. A linkage is possible, only if all of the
connectors are connected, only if they are connected in a way that
preserves the connector types (so that the two endpoints can actually
talk to one-another.)

This kind of capability is not needed for the Internet, or for
peer-to-peer networks, which is why you don't see this "in real life".
That's because humans and sysadmins and software developers are smart
enough to figure out how to connect what to what, and corporate
executives can say "make it so". However, machine agents and "bots" are
not this smart.

So the aim of this project is to create a sensory-motor system, which
self-describes using Link Grammar-style disjuncts. Each "external world"
(the unix filesystem, IRC chat, a webcam or microphone, etc.) exposes
a collection of connectors that describe the data coming from that
sensor (text, images ...) and a collection of connectors that describe
the actions that can be taken (move, open, ...) These connector-sets
are "affordances" to the external world: they describe how an agent can
work with the sensori-motor interface to "do things" in the external
world.

Autonomous Agents
-----------------
The sensori-motor system is just an interface. In between must lie a
bunch of data-processing nodes that take "inputs" and convert them to
"outputs". There are several ways to do this. One is to hand-code,
hard-code these connections, to create a "stimulus-response" (SRAI)
type system. For each input (stimulus) some processing happens,
and then some output is generated (response). A second way is to create
a dictionary of processing elements, each of which can take assorted
inputs or outputs, defined by connector types. Link Grammar can then be
used to obtain valid linkages between them. This approach resembles
electronics design automation (EDA): there is a dictionary of parts
(resistors, capacitors, coils, transistors ... op amps, filters, ...)
each able to take different kinds of connections. With guidance from the
(human) user, the EDA tool selects parts from the dictionary, and hooks
them up in valid circuits. Here, Link Grammar takes the role of the EDA
tool, generating only valid linkages. The (human) user still had to
select the "LG words" or "EDA parts", but LG/EDA does the rest,
generating a "netlist" (in the case of EDA) or a "linkage" (in the case
of LG).

What if there is no human to guide parts selection and circuit design?
You can't just give an EDA tool a BOM (Bill of Materials) and say
"design some random circuit out of these parts". Well, actually, you
can, if you use some sort of evolutionary programming system. Such
systems (e.g. [as-moses](https://github.com/opencog/as-moses)) are able
to generate random trees, and then select the best/fittest ones for some
given purpose. A collection of such trees is called a "random forest" or
"decision tree forest", and, until a few years ago, random forests were
competitive in the machine-learning industry, equaling the performance
seen in deep-learning neural nets (DLNN).

Deep learning now outperforms random forests. Can we (how can we) attach
a DLNN system to the sensori-motor system being described here? Should
we, or is this a bad idea? Let's review the situation.

* Yes, maybe hooking up DLNN to the sensory system here is a stupid
  idea. Maybe it's just technically ill-founded, and there are easier
  ways of doing this. But I don't know; that's why I'm doing these
  experiments.

* Has anyone ever built a DLNN for electronic circuit design? That is,
  taken a training corpus of a million different circuit designs
  (netlists), and created a new system that will generate new
  electronic circuits for you? I dunno. Maybe.

* Has anyone done this for software? Yes, GPT-4 (and I guess Microsoft
  CodePilot) is capable of writing short sequences of valid software to
  accomplish various tasks.

* How should one think about "training"? I like to think of LLM's as
  high-resolution photo-realistic snapshots of human language. What you
  "see" when you interact with GPT-2 are very detailed models of things
  that humans have written, things in the training set. What you see
  in GPT-4 are not just the surface text-strings, but a layer or two
  deeper into the structure, resembling human reasoning. That is, GPT-2
  captures base natural language syntax (as a start), plus entities and
  entity relationships and entity attributes (one layer down, past
  surface syntax.) GPT-4 does down one more layer, adequate capturing
  some types of human reasoning (e.g. analogical reasoning about
  entities). No doubt, GPT-5 will do an even better job of emulating
  the kinds of human reasoning seen in the training corpus.  Is it
  "just emulating" or is it "actually doing"? This is where the
  industry experts debate, and I will ignore this debate.

* DLNN training is a force-feeding of the training corpus down the
  gullet of the network. Given some wiring diagram for the DLNN,
  carefully crafted by human beings to have some specific number of
  attention heads of a certain width, located at some certain depth,
  maybe in several places, the training corpus is forced through the
  circuit, arriving at a weight matrix via gradient descent. Just like a
  human engineer designs an electronic circuit, so a human engineer
  designs the network to be trained (using TensorFlow, or whatever).

The proposal here is to "learn by walking around". A decade ago, the MIT
Robotics Lab (and others) demoed randomly-constructed virtual robots
that, starting from nothing, learned how to walk, run, climb, jump,
navigate obstacles. The training here is "learning by doing", rather
than "here's a training corpus of digitized humans/animals walking,
running, climbing, jumping". There's no corpus of moves to emulate;
there's no single-shot learning of dance-steps from youtube videos.
The robots stumble around in an environment, until they figure out
how things work, how to get stuff done.

The proposal here is to do "the same thing", but instead of doing it
in some 3D landscape (Gazebo, Stage/Player, Minecraft...) to instead
do it in a generic sensori-motor landscape.

Thus, the question becomes: "What is a generic sensori-motor landscape?"
and "how does a learning system interface to such a thing?" This git
repo is my best attempt to try to understand these two questions, and to
find answers to them. Apologies if the current state is underwhelming.

### Status
***Version 0.2.8*** -- Experimental. Basic demos actually work. Overall
low-level parts of the architecture and implementation seem ok-ish. The
upper-level parts have not yet been designed. The grand questions above
remain mysterious, but are starting to clarify.

Provides:
* Basic File I/O stream.
* Prototype Filesystem navigation stream.
* Prototype IRC chatbot stream.
* Basic interactive terminal I/O stream.

See the [examples](examples) directory.

The [AtomSpace Bridge](https://github.com/opencog/atomspace-bridge)
provides an API between the AtomSpace and SQL. It almost conforms to
the system design here, but not quite. It should be ported over to
the interfaces here.

The [Vision subsystem](https://github.com/opencog/vision) provides
an Atomese API for OpenCV. It is a proof-of-concept. It should be
ported over to the interfaces here.

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

* [Design Overview](Design.md) -- Current design & TODO List.
* [IRChatStream](opencog/atoms/irc/README.md) -- IRC chat design.
* [TextFileStream](opencog/atoms/filedir/README.md) -- Directory navigation design.
* [TerminalStream](opencog/atoms/terminal/README.md) -- Interactive terminal design.

### Build and Install
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
See the [examples](examples) directory. The simplest example is for
pinging text between two xterms. Other examples include opening,
reading & writing a single text file, navigating the file system,
and a basic IRC echobot.

***Important*** All of this is pre-alpha! These examples are too
low-level; the intent is to eventually automate the process for hooking
up sensors to motors. Basic design work continues. But for now, these
show some of the low-level infrstructure; the high-level stuff is still
missing.
