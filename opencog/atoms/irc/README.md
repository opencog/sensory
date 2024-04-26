
Internet Relay Chat
===================
IRC chat API. Can connect to IRC, read, and write. Works but still
a rough prototype.

See the [irc-api.scm](../../../examples/irc-api.scm) and
[irc-echo-bot.scm](../../../examples/irc-echo-bot.scm) demos.

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
  increments. The shape of the space is constrained by equivalences (e.g.
  group presentation equivalences that convert a free group into Eucliden
  space). This includes holonomic constraints: the joint-angle space of
  a robot arm.

There is no "continuous movement" in IRC. So punt on that.

### Perception of possibilities
The menu of actions needs to be perceived: one must explicitly "look at"
the menu of possible actions that lead to possible future worlds. To
have a consistent paradigm of what we've done so far, the answer seems to
be: open the object (like a directory listing) and read what's there, and
pick one.

### Designating actions
How do we designate which action to take?  Currently, both IRC and files
are designated with URL's. Three problems:
* How do we get a channel listing? Do we open a URL
  "irc://server/do-channel-list"? Since we're already connected the
  first part of the URL is redundant.
* On receipt of channel listing, how do we join a specific channel?
* We want to avoid string-string concatenation for building custom URL's
  because nothing in Atomese so far does string stuff. Its hypergraphs,
  always. Don't want to invent a string-concatenator.

The conventional object-oriented style is to "send a message". What are
the allowed messages? Chat text messages are free-form. Directory
listings are specific commands chosen from the given command menu.
When opening a connection, how do we know if the things that are read
are command options, vs free-form text input?

Conclude: the format needs to be:
```
	(define irc-stream (cog-execute!
		(Open
			(TypeNode 'IRChatStream)
			(SensoryNode "irc://nick@server/"))))

	irc-stream ; list available commands, e.g. list channels, join
	; Return will be
	(LinkValue
		(ActionNode "help")   ; help menu
		(ActionNode "list")   ; listing of channels
		(ActionNode "join"))  ; channel to join
```
Now what? One obvious mode would be:
```
	(define look-stream (cog-execute!
		(Open
			irc-stream  ; this provides context
			(ActionNode "join")
			(ItemNode "#opencog")))
```
But this has problems: how did we know that the action "join" needs
an argument, and how do we know that the argument must be a valid
choice from the "list" command?

It also overlaps with the other way of doing directives:
```
   (Write
		irc-stream
		(ActionNode "JOIN")
		(ItemNode "#opencog"))
```
OK so here we have a distinction: `Open` is a kind of movement,
taking us to a new place, opening a new stream. `Write` keeps us
on the current stream, simply advancing the time-like cursor/iterator
on that stream.

Still: it's ambiguous. We can use `Open` to create a new stream
dedicated to a specific channel. Or we can use `Write` to alter
the state/mode of the current stream. Writes then make the stream
behave in a modal way, which is perhaps confusing to the agent?

The problem here is that the IRC protocol hides some of this modality:
each message shows the sender and the receiver.  The result is a
multi-threaded stream. The JOIN command is more of a "listen to this"
command.

So two things: the robot-command of "send this message to this device on
this control stream", and the attention-allocation issue of "listen to
this particular thing" or even "pay attention to this, and perhaps
respond to this" as a sub-item in the open channel. Ugh.

Some IRC commands are triplets:
```
   (Write
		irc-stream
		(ActionNode "PRIVMSG")
		(ItemNode "#opencog")
		(String "blal blah text to say out loud"))
```

For now, hand-code these by force.

Disjuncts (Sheaf sections)
==========================
Sheaf sections (aka disjuncts) are a way of indicating partial graphical
networks, having both "closed" portions (edges between vertexes) and
"open" portions (unconnected jigsaw connectors). The unconnected
connectors are offers of future possibilities: hypothetical possible
worlds. The prototype section of computing is the lambda of lambda
calculus: the variables bound by the lambda are the connectors, and
beta reduction is the act of connecting the connectors.

Perhaps disjuncts are the correct way to indicate possible actions.
How might this work? Lets try it.
```
	(Section
		(Item "Name of this disjunct")
		(ConnectorSeq    ; Could also be ConnectorSet
			(Connector
				(Sex "name of first mating rule")
				(Item "type of first connector"))
			(Connector
				(Sex "name of second mating rule")
				(Item "type of second connector"))
```
Conventional sex mating rules for computing are "input" and "output".
Concentional functions in computing have zero or one output connectors,
and zero or more input connectors. The inputs and outputs are typed.
Polymorphic types can be indicated with ChoiceLink, and so on.


---------------------
