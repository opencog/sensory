
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

The basic "action" seems to be "Open", which philosophically is "look
at". The result of "open" is a data stream that can be sampled. But this
data stream is complex: it has structure. If the stream is chat
conversation, then how do we represent the different users talking,
their nicks, hosts, etc.?

What about sub-actions? After taking the action to join a server, one
has several subactions. One is to list all of the channels. Another
action is to select one channel and join it. After joining a channel,
there are several more sub-actions possible: list all of the users,
start a private conversation with a single user, or spew stuff into
the (public) channel.

The sub-actions are modal: they do not become possible or available,
until certain earlier actions have been performed. The menu of possible
actions also changes modally: some actions might disappear over time.

### Actions
What kind of actions are possible?

* Menu selection: This is like a file system directory listing; a
  discrete list of options are presented. The result of selecting
  from that list returns an open connection/file-handle, which can
  be read, and maybe can be written to.

Constraints and equivalences are ad-hoc: sometimes menus change,
sometimes they don't. Some things are still possible after taking an
action; other things become impossible. This is not like a free group
(where anything is possible at any time) nor is it like a group
presentation (where what can currently be done is described by a set of
equivalences, possibly commutative equivalences.) There are no a priori
equivalences.

* Continuous space: Movement in a space with arbitrarily small movement
  increments. The shape of the space is contrained by equivalences (e.g.
  group presentation equivalences that convert a free group into Eucliden
  space). This includes holonomic constraints: the joint-angle space of
  a robot arm.

There is no "continuos movement" in IRC. So punt on that.

### Perception of possibilities
The menu of actions needs to be perceived: one must explicitly "look at"
the menu of possible actions that lead to possible future worlds. To
have a consistent paradigm of what we've done so far, the answer seems to
be: open the object (like a directory listing) and read whats there, and
pick one.
