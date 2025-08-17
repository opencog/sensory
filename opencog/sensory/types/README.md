
Sensory Atom Types
------------------
This defines several SensoryNode types.
Some of the current and envisioned data stream types include:

* `TextFileNode` -- read and write files and directories.
* `IRChatNode` -- IRC chatbot connection
* `TwitterNode` -- Twitterbot Node
* `DiscordNode` -- Discord bot Node
* Vision & sound: see the [opencog/vision](https://github.com/opencog/vision)
  git repo.

Experimental. Not sure how this will work out.

The TwitterNode & DiscordNode are currently a daydream.
The vision stream exists as a proof-of-concept, implementing a wrapper
around OpenCV.

General information about how Atom types are specified, and the format
of the `sensory_types.script` file, can be found in the
[AtomSpace atom_types](https://github.com/opencog/atomspace/tree/master/opencog/atoms/atom_types)
directory.
