
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
networks, having both "closed" portions (edges between vertices) and
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
				(Item "type of first connector")
				(Sex "name of first mating rule"))
			(Connector
				(Item "type of second connector")
				(Sex "name of second mating rule"))
```
Conventional sex mating rules for computing are "input" and "output".
Concentional functions in computing have zero or one output connectors,
and zero or more input connectors. The inputs and outputs are typed.
Polymorphic types can be indicated with ChoiceLink, and so on.

How can this be used to list environmental situations? Goal is to
represent: "here's what yo can do: you can list all members of the chat
channel, and you can open a private chat to one of them, and you can
maybe kick or ban or give voice to one of them, but only if you are
channel op."

So, given a location in an environment, one has available:
```
	(ChoiceLink
		(Section ...) ; possible action one
		(Section ...) ; possible action two
   ...)
```

Then, "list users" would be
```
	(Section
		(Item "Informal name of this command is list users")
		(ConnectorSeq
			(Connector
				(Sex "output")
				uuhhh
```
The output connector needs to describe the format of the command that
this particular sensory stream can actually understand. That is, the
data that the agent must send (using `(cog-execute! (WriteLink ...))`)

What about the contents of a data stream? Say, users joining and
leaving? How do we describe what the sensory stream might deliver?
At open time? Inline with the stream?

If you connect to data stream XYZ, what is the format of the messages
arriving on data stream XYZ?  We can steal some of the old (ancient)
Web 3.0 ideas... e.g DTD definitions, where DTD == [Data Type
Definition](https://en.wikipedia.org/wiki/Document_type_definition).
The oversimplified example DTD for HTML is:
```
<!ELEMENT html (head, body)>
<!ELEMENT p (#PCDATA | p | ul | dl | table | h1|h2|h3)*>
```
We need a DTD-like description of the data flowing on the data stream.

### Syntactical parsing for sensory-motor action chains
Now comes the interesting part. We have a DTD-like data descriptor
of what is being sent out on a sensory stream. What happens after
that data has been transformed a few times, e.g. by FilterLinks
with RuleLinks?

In principle, we can look at the RuleLink and infer what has happened
to the data. This is coneventiona proof-theory of chaining together
inference steps.

For a chain of steps, we want an inference engine that says: "after
perceiving sensory info of type DTD and applying a sequence of
transforms given by RuleLinks, we can infer that the output type is a
DTD of type foo. And motor action controller will accept DTD's of
type foo, and so it is possible to hook up this entire processing
chain. i.e. it is 'parsable'".

Parsable, in the sense that connectors provided by the sensory stream
can be attached to connectors inferred from FilterLinks & RuleLinks,
and that those connectors are, in turn, joinable to the output device.

So we want to describe inputs and outputs as disjuncts and allow LG
style parsing to determine what the syntactically valid hookups are.

### Linkage examples
So lets try to work through an example. We do this:
```
   (cog-execute! (Lookup (Type 'IRChatStream)))
```
The above is an "action". It returns a "perception", which is a menu
choice of actions that can be taken. So this is like the first-ever
computer game "Adventure": "you are standing in front of..."
```
	(ChoiceLink
		(Section (Item "Open") ...)  ; Description of stream open
		(Section (Item "Write") ...) ; Description of stream write
   ...)
```
This is analogous to a LinkGrammar dictionary lookup, where the "word"
is `(Type 'IRChatStream)` and the set of disjunts on the "word"
are Sections.  Note that the available sections will be independent
of which URL is being opened.

What does the first Section look like, in detail?
```
(Section
	(Item "this is our Open command")
	(ConnectorSeq
		(Connector
			(Sex "command")        ; Command means it is sent.
			(Type 'OpenLink))      ; What to send (must be OpenLink)
		(Connector
			(Sex "command")        ; Send
			(Type 'SensoryNode)))) ; What to send, i.e. the URL to open
```
The above is supposed to describe the ability to open an `IRChatStream`
and specifically, the required arguments for the `Open`.  The net
result is that we want to do this:
```
(cog-execute!
   (Open
      (Type 'IRChatStream)
      (SensoryNode "irc://botty@irc.libera.chat:6667"))))
```
Not yet clear how the Section that described the ability to open a
connection actually causes the `OpenLink` to be executed... Hmm

Anyway, performing the `Open` action results in a new menu of disjuncts
being provided:
```
	(ChoiceLink
		(Section (Item "Join") ...) ; Description of the JOIN IRC command
		(Section (Item "List") ...) ; Description of the LIST IRC command
   ...)
```
This is provided as the return value from the execute. What does the
Join command description look like?
```
	(Section
		(Item "This is what channel JOIN messages are like")
		(ConnectorSeq
			(Connector
				(Sex "command")      ; This connector is sent-to IRC
				(Type 'WriteLink))   ; Use a WriteLink to send.
			(Connector
				(Sex "command")      ; This connector is sent-to IRC
				(Item "JOIN"))       ; String to be sent
			(Connector
				(Sex "command")      ; This connector is sent-to IRC
            (Type 'StringValue)) ; Channel name must be provided.
			(Connector
				(Sex "reply")        ; Stream of returned values
				(LinkSignature       ; Return will be ...
					(Type 'LinkValue) ; ... a list of Values ...
					(Type 'StringValue))) ; ... that are nicknames (strings)
		))
```
The above is loosely typed, so that the nicknames are returned as
strings. Could be strongly typed, to indicate nicknames are being
returned, and elsewhere we know nicknames are actually strings.

Say we wanted to write the nicknames to a file. We would do this
in a fashion similar to the above:
```
   (cog-execute! (Lookup (Type 'FileStream)))
```
which returns
```
	(ChoiceLink
		(Section (Item "Open") ...)  ; Description of stream open
		(Section (Item "Write") ...) ; Description of stream write
   ...)
```
The file-open command looks just like the IRC open command, except
that we need to give a `file://` URL instead. The write command
describes write in such a way that we know that we can write strings.
But since the channel join command output was a list of nicknames
which happen to be strings, we know that we can write strings to
the file.

This glosses over several things. These are:
* The File API needs to be more complicated, because we need to be able
  to walk up and down directory trees.
* We seem to be confusing bare connectors with descriptions of the
  data that will flow through that connector, once the link is joined
  (and the stream is flowing).
* We haven't explained how to actually join stream outputs to inputs.
* Unlike Link Grammar, where all words are known a priori, here, we do
  not find out what the possible actions are, until we arrive at that
  "location". i.e. we don't know, until we perform that action, and
  then get the menu of places we can go to, from there.

---------------------
