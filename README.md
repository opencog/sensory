Sensory Atomese
===============
This repo explores how perception and action within an external world
might work with the [AtomSpace](https://github.com/opencog/atomspace).

TL;DR: Explores philosophical approaches to perception & action via
actual, working code using low-level AtomSpace sensory I/O Atoms.
A software conception of "basal cognition".
The experimental lab for this is "perceiving" filesystem files,
"moving" through directories, and likewise for IRC chat streams.

This is part of a group of inter-related research projects:
* [Agents](https://github.com/opencog/agents), which aims to make use
  of the structures developed here.
* [Motor](https://github.com/opencog/motor), which looks at the issues
  raised below from a different perspective, perhaps simplifying the
  problem into a more tractable and practical form.

Motivation
----------
OpenCog has repeatedly attempted to build "embodied AI systems", by
attaching symbolic and probabilistic AI reasoning systems to assorted
robot platforms. These include:
* The [Hanson Robotics](https://www.hansonrobotics.com/) Sophia robot,
  via [ROS and Blender](https://github.com/opencog/docker/tree/master/indigo).
* [Minecraft](https://www.minecraft.net), via MineRL and Malmo, with the
  Rational OpenCog Controlled Agent,
  [ROCCA](https://github.com/opencog/rocca).
* The Unity game engine, to create
  [pet](https://www.youtube.com/watch?v=FEmpGRLwbqE)
  [dog](https://www.youtube.com/watch?v=vZtnjKcrdZQ)
  [avatars](https://www.youtube.com/watch?v=of-BahzS8qQ)
  that
  [talk](https://www.youtube.com/watch?v=ii-qdubNsx0).

These are the major efforts; there were half-a-dozen lesser efforts,
including a soccer-playing robothon, held in Ethiopia. All of these
failed, although all were educational: the robothon was explictly
arranged by students in Ethiopia as a part of university coursework.

All of these failed, in large part because not enough effort was
put into understanding sensing and motion, and far too much on
reasoning, planning and language. The ideas of sensing and movement
seem trivial and obvious, and are implemented with brute-force hackery.
Not worthy of intellectual effort, in contrast to the veneration
given to reasoning and logic.  This is a fundamental mistake.


Philosophical Overview
----------------------
The issue for any agent is being able to perceive the environment that
it is in, and then being able to interact with this environment.

The goal in this project is to find a theory of a minimum viable
low-level sensory API. Having such a theory would clarify how learning
systems might learn how to use it, or, more generally, how to
"use things", how to have a cause-and-effect in the universe.

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

Currently, four interfaces are being explored: a basic text-terminal,
a single-file reader/writer, a unix filesystem navigator, and an IRC
chat interface. Hopefully, this is broad enough to expose some of the
design issues. Basically, chat is not like a filesystem: there is a
large variety of IRC commands, there are public channels, there are
private conversations. They are bi-directional.  The kind of sensory
information coming from chat is just different than the sensory
information coming from files (even though, as a clever software
engineer, one could map chat I/O to a filesystem-style interface.)
The point here is not to be "clever", but to design action-perception
correctly.  Trying to support very different kinds of sensorimotor
systems keeps us honest.

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
Other examples include [Interface Description
Languages](https://en.wikipedia.org/wiki/Interface_description_language),
the X.500 and LDAP schemas, as well as SNMP.

However, there is no generic way of asking a pipe "hey mister pipe, what
are you? What kind of data passes over you?" or "how do I communicate
with whatever is at the other end of this pipe?" Usually, these
questions are resolved by some sort of hand-shaking and negotiation
when two parties connect.

The experiment being done here, in this git repo, in this code-base, is
to assign a type to a pipe. This replaces the earliest stages of
protocol negotiation: if a system wishes only connect to pipes of type
`FOO`, then it can find out what is available by examining ("looking
at") the pipe description. The pipe descrription is a disjoint list
of connector types: types that caharacterize how a connection can be
made.  If this list is
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

Wiring
------
The words "wiring", "wire up" and "connect up" are being used
self-consciously.  This is what one does for electrical and electronic
circuits; but it is also what is done for plumbing, say, for a chemical
refining plant. One distinguishes the devices connected up, from the
current flowing on the wires. The current itself has properties:
different voltages, or, in the case of chemical processing, different
substances.

Wiring is conventionally done by specifying netlists: lists of what is
connected to what, organized by type of wire. There's a multi-billion
dollar industry dealing in
[Electronic Design Automation (EDA)](https://en.wikipedia.org/wiki/Electronic_design_automation)
tools. Prominent programming languages include
[VHDL](https://en.wikipedia.org/wiki/VHDL) and
[Verilog](https://en.wikipedia.org/wiki/Verilog) for digital circuits,
and
[SPICE](https://en.wikipedia.org/wiki/SPICE) for analog circuit design.
Verilog has been adapted for mixed-signal design, and also for
[synthetic biological circuits](https://en.wikipedia.org/wiki/Synthetic_biological_circuit),
and so is relatively generic.

This project is explicitly performing a kind of wiring, but it is using
Atomese, not Verilog, for that wiring. Could this project be done in
Verilog? Possibly. When Atomese was being invented, this wasn't
foreseen.

There is another example where "wiring" is commonly done: in
[compilers](https://en.wikipedia.org/wiki/Compiler). A compiler takes a
high-level language (C, C++, Java) and converts it to
[assembly code](https://en.wikipedia.org/wiki/Assembly_language).
Part of the magic of doing this is to describe CPU hardware, and
specifically the assembly instructions, as if they were electronic
devices, having inputs and generating outputs. The assembly instructions
are then "wired up", so that data flows correctly through them. Thus, for
example, the ADD instruction has two inputs, one output, all of which
are registers; the result must go into the register that the next
instruction is expecting, or must be routed to memory.

Examples of such data-flow descriptions include gcc's
[Register Transfer Language (RTL)](https://en.wikipedia.org/wiki/Register_transfer_language)
and [GIMPLE](https://gcc.gnu.org/onlinedocs/gccint/GIMPLE.html).
There are many more: the idea of an
[intermediate representation (IR)](https://en.wikipedia.org/wiki/Intermediate_representation)
language is generic in programming language design.

Just like Verilog is intended for electronic circuits, the IR languages
are intended for programming languages. Neither of these generalize very
well to domains outside of their original specification. Atomese is
attempting to be the superset or generalization of all of these
different approaches to wiring. It is trying to capture the generic
abstraction of "what is wiring" and "what does it mean to hook things
together".

Two side-comments: it is not an accident that Atomese resembles GCC's
RTL. Both capture something fundamental about wiring. There is also
another tickling analogy: Atomese Values are meant to be transient
changing things, while Atoms in the AtomSpace are meant to be static,
with the AtomSpace a repository, a database for these Atoms. This
resembles the relatioship between CPU registers, where computation takes
place, and system RAM, while holds and "remembers" stuff. One of the
basic rewrite rules in Atomese is the one that moves Values into Atoms,
storing them in the AtomSpace, and vice-versa, streams Atoms out of the
AtomSpace, and into Values.

Some generalizations and generalities: in chemistry, one has literal
atoms, which, like tinker toys or jigsaw-puzzle pieces, can hook up one
to the other, to form complex molecules. The process is recursive:
certain complex molecules, such as DNA and protiens, can further hook up
and participate in processing. The idea of hooking up and connecting is
pervasive. It's in chemistry certainly, and obviously in mechanical
linkages. One builds high-rises by connecting steel girders.

One of the sub-goals of this project is to understand the generic,
mathematical, philsophical nature of what it means to "hook things up",
in full abstractness.


Auto-wiring and theorem proving
-------------------------------
The goal of describing system blocks with some DTD or IDL language,
and then hooking them up "automatically", is not new. Four approaches
are worth mentioning.

* [ProLog](https://en.wikipedia.org/wiki/Prolog) and
  [Answer Set Programming (ASP)](https://en.wikipedia.org/wiki/Answer_set_programming).
  In these systems, one asserts a collection of facts/assertions, and
  then says "let's go". Prolog uses a chaining algorithm, and ASP uses
  a [SAT solver](https://en.wikipedia.org/wiki/SAT_solver) to determine
  a "solution" satisfying the constraints embodied in the facts. One
  does _not_ get back the actual inference chain that was used. For
  this project, we want to know *how* things got hooked together.
  We're not interested in a true/false satisfiability answer, but
  rather in finding out how assemble a processing chain.

* [Automated theorem provers](https://en.wikipedia.org/wiki/Automated_theorem_proving).
  These are systems that, given a set of facts/assertions, together
  with a collection of inference rules, provide an actual inference
  chain, aka a "proof", that explicitly gives the steps from the
  hypothesis (the input claims) to the result (that the input claims
  are consistent and simultaneously satisfiable.) This gets closer to
  what we want: we want the chain. However, unlike theorem provers,
  we're not particularly interested in satisfiability, other than to
  find out if a partially-assembled pipeline still has some missing
  end-points. This is why we work with sheaves or jigsaws: we want to
  know not only how to assemble the jigsaw pieces, but to also know
  what the remaining, open (unconnected) connectors are. Theorem
  provers do not provide such info.

* The [SOAR Cognitive Architecture](https://en.wikipedia.org/wiki/Soar_%28cognitive_architecture%29).
  This is a production system that applies production rules to state.
  This captures a different aspect of what we want to do here: we want
  to have data, and to apply rules to that data to transform it.
  However, our data is not so much "state" as it is a "stream": think
  of an audio or video stream. SOAR selects a rule (analogous to an
  inference rule, in theorem proving) and applies it immediately, to
  mutate the state. By contrast, we want to think of inference rules as
  jigsaw pieces: how can they be assembled? Once assempled, then these
  rules can be applied to not just "state" in a single-shot fashion,
  but repeatedly, to a flowing stream of data (say, video, to find all
  cats in the video).

* [Programming language compilers](https://en.wikipedia.org/wiki/Compiler).
  These are able to take high-level specifications and convert them
  into an equivalent program written in
  [assembly language](https://en.wikipedia.org/wiki/Assembly_language).
  The assembly instructions can be thought of as jigsaws, and when they
  are assembled, the mating rules must be closely followed: the output
  registers in one instruction must attach to the inputs of another.
  The resulting program has to be runnable, executable. This is exactly
  what we want here: an assembly of interconnected jigsaws. However,
  unlike a compiler, our "assembly language" consists of various
  abstract processing components: filters, transforms, etc. It doesn't
  run on a (real or virtual) CPU, but on an abstract machine. It is not
  registers and RAM that the instructions/jigsaws act on, but on
  sensory data (again: think video/audio).  Worse yet, the instructions
  aren't even fixed: new ones might get invented at any time. These
  might supplement or replace old ones. Compilers also want a program,
  written in a high-level language, as input. In this project, we won't
  have such a program; our situation is closer too SOAR or ProLog or
  theorem provers: we have a collection of jigsaws (instructions) to
  assemble, but no high-level program to specify that assembly.

The above systems solve some of the aspects of what we want to do here,
but only some of them, and not in the format that we actually need.
The above should give a flavor of why we're embarked on the crazy
journey we're on. No one else does this.


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

(Footnote:
[Stimulus-response](https://en.wikipedia.org/wiki/Stimulus%E2%80%93response_model)
is an old concept in psychology, dating back to Pavlov. SRAI is the name
given to the rules used by the
[AIML](https://en.wikipedia.org/wiki/Artificial_Intelligence_Markup_Language)
chatbot language. The terminology is not accidental.)

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

(Footnote: A collection of items, each given a score, is termed an
["ensemble"](https://en.wikipedia.org/wiki/Ensemble_(mathematical_physics))
in statistical physics. Thus, a
[random forest](https://en.wikipedia.org/wiki/Random_forest),
where each tree is assigned a real-number fitness score, is an
ensemble of trees.  Ensembles are described by
[partition functions](https://en.wikipedia.org/wiki/Partition_function_(mathematics)):
roughly, functions that tell you how many items there are having a given
score. Partition functions tend to have a Gaussian (Bell curve)
distribution, with the mean of the curve given by an
[Action](https://en.wikipedia.org/wiki/Action_principles).
This last is highly technical: the action describes not only the mean
but also how neighboring items are related.  When the ensemble forms a
continuum, the action can be used to obtain equations of motion; these
are the
[Hamilton-Jacobi equations](https://en.wikipedia.org/wiki/Hamilton%E2%80%93Jacobi_equation)
There is a very rich theory surrounding these, everything from geodesics
on Riemann surfaces to quantum field theory, with applications ranging
from chemical reaction rates to petroleum exploration to
measure-preserving dynamical systems. It is no accident that Bill Friston
proposes free energy as the fundamental underlying theoretical principle
that can be used to understand AGI or general intelligence.  The
ensemble is a powerful concept, and when coupled to a fitness score and
the accompanying mathemetcial apparatus of Gibbs free energy and
Boltzmann distributions, it aappears to be pervasive. The issue here is
that we are still grasping at basic principles: the AtomSpace and Atomese
allow for ensembles of representations of "mechanical parts" to be
created, but it is not yet clear how to score them, how to create the
Ising-like model of interacting, self-assembling components. We are
still very far away from being able to write down a generic action for
Atomese.)

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


Self-observing systems
----------------------
Perception need not be limited to "the external world"; one may also
observe oneself. Action need not be limited to the movement of limbs; it
can also be a control over one's own thoughts. Imagine the case of
"stewing in one's own juice's": a dreamlike state, where you ruminate
over old memories of pst events. As one does so, one selects, picks and
chooses: fond memories are noted for their emotional content, are given
further thought; boring memories stay unexamined, unless forced.

The perception-action system described above can be aimed not just at
the external world, but also at internal state. Imagine, for example, a
trained LSTM or maybe an LLM that is placed into a "dream state", where
it generates a sequence of free-association outputs. An agent can watch
over this stream, and, in response to certain outputs, it can "prompt"
the system, to guide further "reminiscences". More directly, it can
"choose" to force the system to concentrate on a specific topic. To
"focus one's thoughts". This kind of hierarchical layering, where an
agent steers the thoughts of an underlying system results in a form of
self-awareness and self-control.

Thus, in addition to the previously-described perception-action agents
(e.g. traversing the file system or interacting via chat) one can build
an agent that monitors the state of a neural network, and then controls
it, via prompts, or perhaps much more directly with gates (tanh/signmoid
blending.)


Related ideas
-------------
A distantly related set of ideas can be found in the [SOAR Cognitive
Architecture](https://en.wikipedia.org/wiki/Soar_%28cognitive_architecture%29)
from Laird, Newell & Rosenbloom. Examples of SOAR agents can be
seen in the [SOAR Agent github repo](https://github.com/SoarGroup/Agents/).
SOAR is a production rule system, and so resembles the AtomSpace query
subsystem. The manner in which SOAR rules are applied resembles the
[OpenCog Unified Rule Engine (URE)](https://github.com/opencog/ure)
(which is unsupported and now deprecated) and also
[OpenCog Probabilistic Logic Networks (PLN)](https://github.com/opencog/pln)
(also unsupported & deprecated).

There are many differences between this work and Atomese:
* SOAR production rules are written in ASCII (in the SOAR language), and
  are stored in flat files.  AtomSpace production rules are written in
  Atomese, and stored in the AtomSpace.
* SOAR production rules are applied to "state", and work within a
  "context". This is similar to the AtomSpace, which can store state.
  However, the AtomSpace can also store the rules themselves (as
  "state") and the rules can be applied to external data streams
  (e.g. audio, video).
* SOAR production rules are crafted by humans, encoding knowledge.
  Atomese rules are meant to be algorithmically generated and
  assembled.
* SOAR state mutation is boolean-valued: either something is done, or
  it isn't. There does not appear to be any concept of Bayesian
  possible-worlds.
* SOAR appears to use a very simplistic forward-chaining approach to
  inference. For any given SOAR state, a collection of possible rules
  is determined. Of these, one rule is selected, and then it is
  applied to mutate the state. It has long been recognized that other
  kinds of chaining is interesting: not just forward chaining, but also
  backwards. One might not just chain (as in ProLog), but ask for
  constraint satisfaction (in ASP and automated theorem provers.)
* The OpenCog URE and PLN elements are capable of performing chaining.
  They are currently deprecated/obsolescent, for a variety of technical
  and philosophical reasons.

A nice, quick & easy overview of SOAR can be found here:
"[An Introduction to the Soar Cognitive
Architecture](https://acs.ist.psu.edu/ist597/pst-soar%20v14.2.pdf)",
Tony Kalus and Frank Ritter (2010)

### CGW Wires
There was a much earlier attempt at wiring with Atomese, from 2008,
termed "Cog Graphical Wires" (CGW). It never went anywhere. Right
idea, wrong time.  The description still sounds sexy, and mirrors
the above. It can be found in the now-deleted directory
[CGW Wires](https://github.com/opencog/atomspace/tree/3f58a2cdd7891da074ee48bd517c7f656ff12b14/opencog/scm/wires)
That code was inspired by a paper:
* ''The Art of the Propagator'', Alexey Radul; Gerald Jay Sussman,
  MIT Technical Report MIT-CSAIL-TR-2009-002
  http://dspace.mit.edu/handle/1721.1/44215

I haven't read that paper in over a decade. Perhaps it has some gems.


### Status
***Version 0.3.1*** -- Experimental. Basic demos actually work. Overall
low-level parts of the architecture and implementation seem ok-ish. The
upper-level parts have not yet been designed. The grand questions above
remain mysterious, but are starting to clarify.

Provides:
* Basic interactive terminal I/O stream.
* Basic File I/O stream.
* Prototype Filesystem navigation stream.
* Prototype IRC chatbot stream.

The [Architecture Overview](Architecture.md) provides a more detailed
and specific description of how the system is supposed to look like, and
how it is to work, when it gets farther along. The
[Design Diary](Design.md) documents the thought process used to obtain
code that actually works and does what it needs to do.

See the [examples](examples) directory for working examples.

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

The [Architecture Overview](Architecture.md) provides a detailed
description of how this can work.  A general overview can be found
in the AGI 2022 paper:
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

* [Architecture](Architecture.md) -- Architecture overview.
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

It will probably be useful to read the
[Architecture Overview](Architecture.md) first.

***Important*** All of this is pre-alpha! These examples are too
low-level; the intent is to eventually automate the process for hooking
up sensors to motors. Basic design work continues. But for now, these
show some of the low-level infrastructure; the high-level stuff is still
missing.
