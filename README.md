Sensory Atomese
---------------
Implementation for low-level AtomSpace sensory I/O Atoms.
Provides a basic file I/O stream, and an IRC chat stream.

See also the git repo [opencog/vision](https://github.com/opencog/vision)
for OpenCV-based vision Atomese. (Note: It is at version 0.0.2)

### Status
Version 0.2.0 -- Experimental. Starting to gel. There might be more
design changes ahead.

Provides:
* Basic File I/O stream.
* Prototype IRC chatbot stream.

See the [examples](examples) directory.

### Overview
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
