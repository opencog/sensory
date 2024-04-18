Sensory Atomese
---------------
Implementation for low-level AtomSpace sensory I/O Atoms.
Almost nothing here yet, see Status below.

See also the git repo [opencog/vision](https://github.com/opencog/vision)
for OpenCV-based vision Atomese. (It is also at version 0.0.2)

### Status
Version 0.0.2 Super experimental. Not sure how this will work out.

### Overview
The Atomese agent framework needs to have some way of interacting
with it's environment. Obviously, reading, writing, seeing, hearing.
More narrowly: the aboloty to read a text file in the local file system.
The ability to read directory contents, to move throuogh directories.
The ability to behave as a chatbot, e.g. on IRC, but also as a
javascript chatbot running in a web-page. Also possibly running
free on twitter, discord, youtube.

The goal here is to prove the very lowest layers, just the glue,
to convert that stuff into Atomese Atoms that higher-layer Atomese
agents make use of to comminicate with, interact with the external
world.

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
