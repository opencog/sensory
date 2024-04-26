
Sensory Atom Types
------------------
This defines several stream types, and the OpenLink and WriteLink for
constructing instances of these streams, and writing content to them.
Some of the current and envisioned data stream types include:

* `TextFileStream` -- read and write files and directories.
* `IRChatStream` -- IRC chatbot connection
* `TwitterStream` -- Twitterbot Node
* `DiscordStream` -- Discord bot Node
* Vision & sound: see the [opencog/vision](https://github.com/opencog/vision)
  git repo.

Experimental. Not sure how this will work out.

The TwitterStream & DiscordStream are currently a daydream.
The vision stream exists as a proof-of-concept, implementing a wrapper
around OpenCV.

General information about how Atom types are specified, and the format
of the `sensory_types.script` file, can be found in the
[AtomSpace atom_types](https://github.com/opencog/atomspace/tree/master/opencog/atoms/atom_types)
directory.
