Examples and Demos
------------------
The examples and demos here show how to use the sensori-motor system.
The first set of examples show the basic API, while the second set wires
them up to a hand-crafted stimulus-response pipeline, to create simple
agents.

While going through these demos, please keep in mind the project goal.
This is to craft a sensori-motor API that will be easy for learning
systems to use to perceive and navigate through the "external" world.

The hand-crafted stimulus-response pipelines are meant to show the
general usage pattern that a learning system would learn; its a target
that the learning system can aim at.

The API is meant to be an affordance for machine learning, and not for
human programmers. If you just wanted to work with files or chat on IRC,
you need none of this stuff. All this complicated glop is meant to allow
simple & easier machine interaction.

### API Basics
Examples that demonstrate how the API works, and the most basic usage.

* `file-read.scm` -- stream file contents to StreamValue
* `file-write.scm` -- stream Atoms/Values to a file.
* `irc-api.scm` -- Demo of connecting to IRC and interacting.

### Agent demos
Examples showing how prototype agents can be built up in Atomese.
These attach the API's (from the previous examples) to Atomese
processing pipelines to build crude stimulus-response agents.

* `irc-echo-bot.scm` -- IRC echo bot demo.
* `filesys.scm` -- Demo of navigating a filesystem.

The ultimate design goal is to replace the crude hand-crafted
stimulus-response pipelines with a system that learns how to use
the sensori-motor interfaces offered here, and use those interfaces
to navigate the "external" world.

The demos show what a stimulus-response pipeline looks like, so
that learning algos can target that general structure.

### Coding hints
Collection of coding hints.

* `hints.scm` -- Using ExecutionOutputLink and GroundedSchema
