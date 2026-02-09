Examples and Demos
------------------
The examples and demos here show how to use the current sensori-motor
system.

The first set of examples show the basic API, while the second set wires
them up to a hand-crafted stimulus-response pipeline, to create simple
agents.

While going through these demos, please keep in mind the project goal.
This is to craft a sensori-motor API that will make it easy for learning
systems to explore, perceive and navigate through the "external" world.

The hand-crafted stimulus-response pipelines are meant to show the
general usage pattern that a learning system would learn; its a target
that the learning system can aim at.

The API is meant to be an affordance for machine learning, and not for
human programmers. If you just wanted to work with files or chat on IRC,
you need none of this stuff. All this stuff is complicated and
convoluted; it is meant for simple & easy machine interaction, rather
than application development.

That is to say, all of these examples are much too complicated and
difficult. The eventual goal is to automate the hooking up of sensors to
motors. That automation layer is completely missing, just right now.
So instead, the list of examples below demo a big complex mess of the
guts of the system.  If you think this is fugnuts complicated, you're
right, it is.

The [Architecture Overview](Architecture.md) provides a sketch of how
things should work, eventually (if that day ever comes).

### API Basics
Examples that demonstrate how the API works, and the most basic usage.

* `file-read.scm` -- Stream file contents to StreamValue
* `file-write.scm` -- Stream Atoms/Values to a file.
* `xterm-io.scm` -- Stream Atoms/Values from/to an interactive terminal.
* `irc-api.scm` -- Demo of connecting to IRC and interacting.
* `filesys.scm` -- Navigate and explore the filesystem.
* `ollama-api.scm` -- Send and receive messages from Ollama.

### Agent demos
Examples showing how prototype agents can be built up in Atomese.
These attach the API's (from the previous examples) to Atomese
processing pipelines to build crude stimulus-response agents.

* `xterm-bridge.scm` -- Copying text between two xterms
* `irc-echo-bot.scm` -- IRC echo bot demo.
* `ollama-bot.scm` -- Ollama responds to IRC messages.
* `parse-pipeline.scm` -- A complicated pipeline processing demo.
* `introspect.scm` -- (Under construction; broken)

The ultimate design goal is to replace the crude hand-crafted
stimulus-response pipelines with a system that learns how to use
the sensori-motor interfaces offered here, and use those interfaces
to navigate the "external" world.

The demos show what a stimulus-response pipeline looks like, so
that learning algos can target that general structure.

Additional demos are provided in the
[Motor Experiments](https://github.com/opencog/motor)
repo. They are there, not here, because they skip over the
connector-mating idea that is central to this repo. Instead of
using connectors, those demos are just hand-wired.
