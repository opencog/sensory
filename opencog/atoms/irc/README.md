
Internet Relay Chat
-------------------
IRC chat API. Can connect to IRC, read, and write. Works but still
a rough prototype.

See the [irc-chat.scm](../../../examples/irc-chat.scm) demo.

Perception and Action
---------------------
IRC offers a non-trivial environment with which an agent can interact
with. There are several kinds of stimulus: multi-party public
conversations in group channels; private conversations with individuals,
and painful events, like getting kicked. Several actions are possible:
one can look at/ask for a listing of all the channels on a server;
one can look at/ask for a listing of all users in a given channel.
This splits into two parts: performing the action of looking, and then
sensing the resulting data/reply. After the sensing comes the
perception, of processing what the all means (done upstream from here.)

Design Issues
-------------
What's the right way to do the above, using Atomese Atoms and Values?
what is the API that is natural for an agent to work with? I don't know.
