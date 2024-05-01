Design Notes Part C -- Filesystem
=================================
***Late April 2024***

File and Directory Navigation - Navigating through directories and
interacting with files. Exploration of using a Link Grammar formalism
for hooking things up.

Perception and Action
---------------------
How does an agent interact with a file system? How can a file system be
exposed, in Atomese, such that an agent can efficiently, easily and
effectively interact with that file system?

This is a beguilingly simple question: files and directories are a part
of software development basics. It becomes harder if you imagine
yourself (or a robot) locked into a virtual universe where files and
directories are all that there are. How do you walk about? Where is your
current position? How do you decide what to do next? Should you read a
file, or write it? Should you read it again and again? Perhaps you
should avoid places that you've visited before. Is there an explore vs.
exploit task here, where one could "exploit" interesting files, vs.
"explore" to find other interesting things?

Wrapped up in the above are low-level and high-level questions. At the
lowest level, we simply want to create an API that an agent can use to
explore the file system. That'ts what the code here is for.

Ruminations and design criteria are given in
[Design Notes Part B -- IRC](DesignNotes-B.md). The goal is to have
both the IRC and the File System streams as similar as is reasonable,
so that the agent does not need to re-learn entirely new interaction
styles.

Examples
--------
The `TextFileStream` can be used to read and write files. See the
See the [examples](../../../examples) directory.

Link Grammar
------------
OK, Let's take the notes in [Design Notes B](DesignNotes-B.md) to
heart. It says that actions, and the result of those actions, should be
described by a `ChoiceLink` of `Section`s. The
[current filesys example](examples/filesys.scm),
as it stands, treats this as a DTD and attempts to manually pick it
apart and create just one single link. And ... almost works but fails.
Because it's complicated.  This hooking-up of things needs to be
abstracted and automated. How?

Part of the problem is that theres a DTD for the filesys commands,
but no matching connector-sets describing the pipeline. So we really
want:
* An API for terminal I/O
* An LG-style DTD for the terminal I/O
* A linker that can construct a pipeline from the Filesys 'ls' command
  to the terminal. The linker auto-connects, so that none of the icky
  code in the demo would need to be written.
* Repeat above, for IRC. Terminal I/O is easier, cause we can punt on
  channel joins, for now. Alternately, could build a pipe from 'ls' to
  a file, but that is lesss fun, because files are not interactive.

WTF is this? The 1960's? I'm reinventing terminal I/O? Really? Didn't
someone else do this, 60 years ago? Am I crazy?

Another task is to pipe the output of the IRC channel list command to a
file (or a terminal), or to create a spybot, that reads IRC channel
convos and writes them to a file.

Can this be done with a single linkage, or are multiple linkages needed?
The IRC echobot seems to require multiple linkages. It reads, then
applies some decision filters to figure out if it is on a public or
private channel, then it applies one of two different kinds of
responses, depending on which, and finally, it writes. So a grand total
of six elements and two linkages:

* Read target
* Public/private decision filter
* Two different reply formulators
* If on public, then only when addressed filter
* Write target

Try it in ASCII graphics
```
                         x --> pub-maybe --> pub-reply -x
                        /                                \
   Read --> pub/priv --x                                  x--> Write
                        \                                /
                         x ----> private reply >--------x
```
Each of the dashed lines is an LG link; the link-type is NOT noted.
The data flow is mostly text. However, the data to indicate if a
messages came via public or private channel, how does that work?
Does it come as a distinct data type flowing on a pipeline? Or
does it manifest into a different connector?  Maybe both, so that
there is a multiplexor or router node, that routes to different pipes,
depending on the prior message it received?

Am I navel gazing?
------------------
The above is so low level, it feels crazy. Lets take a more careful
look, and see if that's true, if there's a better design or a simpler
design or the whole thing is pointless. So, bullet by bullet.

* Terminal I/O -- Well, of course, if we buy into the rest of the design
  framework here, then of course we need a terminal I/O device.
* Message routing/multiplexing -- This is a central concept in many many
  systems: of course, internet packet routers. Of course, e-mail
  store-and-forward agents. Of course, in electronic circuit design.
  None of them are generic Atomese. If we buy into the rest of the
  design framework here, then of course a multiplexor is needed.

* Atomese vs. other languages. So, for example, a hardware multiplexor is
  written in VHDL or Verilog. Internet routers are written in embedded
  languages. Email routers are generic software. No commonality. How
  about IRC chatbots? How are they designed?

A traditional IRC chatbot can be done in several ways. At lowest-level,
open a socket and read/write it. At the next layer up, find some IRC
handling library (or write your own) like the IRC.cc file in this git
repo. Then write ad hoc C++ code to work with it.  At the next layer up,
find someone else's chatbot scripting toolkit, which is typically a
super-easy toy scripting language that allows complete novices to create
reasonably interesting bots.

Am I creating a clumsy, awkward chatbot scripting language? Maybe. But
it is supposed to be more general than just IRC, its supposed to do the
full action-perception sensory stack.

Why am I using Atomese, instead of, say, straight-up scheme? Well,
because I can save Atomese to the AtomSpace. But why does this matter?
Because I can do introspection on Atomese, and one needs a graph
database to store the introspection results. Also, I keep saying
"introspection" but almost never actually do it. Well, I guess the
pattern-matcher/query engine does this, but its again, very low-level.

If I tried very very hard, I could build a version of Scheme that was
Atomese-like, storable in a graph database, and somewhat easy to
introspct. I'm not sure, I think that is what MeTTa was supposed to be.
Yes, straight-up scheme is a lot more readable than Atomese, and so
perhaps the idea was a straight-up MeTTa woud also be a lot more readable
than Atomese. But its all harder to introspect.

That is, I could write an IRC echobot in scheme. It would be easy: just
a very short read-modify-write loop. But there's no easy way to
introspect this, because there are no attachements, no handles, to
perform actions like "disconnect the read from the modify, and insert
a filter at that location", because programming languages almost never
come with such modifier handles that can be grabbed and manipulated.

This is in contrast to things like Blender, or LEGO mindstorms or any
other graphical programming system, which does explicitly have handles
that can be grabbed, because the GUI needs to expose these to the user,
who will drag-n-drop these with a mouse. I think maybe SmallTalk had
some kind of modules for this. But its too late for SmallTalk.

This is in contrast to internet firewall rules, which are a sequence of
rules to be applied to message data flowing into/out of a specific
computer network.

Which brings us to rule systems, such as Drools, or the Java rules
thing, or the now obsolete URE. Which brings us to the doorstep of
parsing. Which is hy Link Grammar. But this brings us to the doorstep
of the theorem-provers, like HOL or Agda. And so hhere we are...

Are there any programming languages that allow:
* Constructs to be grabbed drag-n-dropped with a GUI, like in
  Mindstorms? (Or the historically older systems e.g. the NeXT GUI
  constructor, or Explorer from SGI)
* Allows data to flow on the pipes? (SGI Explorer, Blender do not;
  the objects are 3D shapes.)
* Allows typed data, in the sense of HTML DTD's ...
* Verifies typed linkages ...
* Provides I/O devices...
* Provide a database with a query language, instead of text files ...

Conclusion: I'm reinventing really ancient concepts (terminal I/O really
is from the 1960's. Unix pipes are from the 1970's, NeXT GUI builder
from the 1980's, SGI's Explorer from the 1990's, Internet firewall rules
from the 1990's ...) but all of these ancient concepts are reworked in
an integrated setting that aims to be generic.

I dunno. I don't see any other way. This stuff nags me, but what else
can I do? No one else seems to have anything even close to being this
generic. The system I'm building is klunky and awkward, but what the
heck. Is there any other path forward?

Conclusion: I'm in a thicket, but there's no turning back. The
ingedients to this recipie are all well, known individually. No one has
combined them in quite this way, before. I don't see any other way.
Onwards.

So that was a moment of doubt. Alas.

ROS
---
Oh, hang on. ROS provides a framework for routing data, hooking things
up and unhooking them, using GUI tools, even, and also a DTD-style
system, using YAML files. It's not Atomese, but its close. And it has a
huge set of supported I/O devices.

Is this what I've missed? Is this what I should be integrating with?
Do I need an Atomese/ROS supernode? Huh. You know what? Yes, I think I
do. I need Atomese/ROS interop.

OK, that will be the next project, but first, I need to bring the
current linking idea into fruition.

Linking
-------
Resume the linkage ideas. There is a text source. There is a text sink.
There  is a command "hookup" to join these two. What does the hookup
command do?
* Verify that one is a source, the other is a sink.
* Verify that the types agree (that both handle text data)

If either end presents itself as a ChoiceLink, and multiple hookups are
possible, then a list of possible linkages are required. So, this is now
parsing.
* Stage zero: get endpoint descriptions/DTD's (as Sections)
* Stage one: parse, propose linkages.
* Stage two: select one or more linkages.
* Stage three: perform the actual hookup, create the network.
* Stage four: execute, start data flowing.

Solution:
* Stage zero: this is what LookatLink does.
* Stage one: build a connector-matcher. This is not a full parser,
  but is a prereq for matching connetors. MatchLink
* Stage two: punt, build a random selector.
* Stage three: build a HookupLink
* Stage four: `cog-execute!` is all we need.

How does MatchLink work?

Worked example of linkage
-------------------------
Lets take the simplest case: two `TerminalStream`. These are the
simplest-possible systems. The have four modes:
* LookatLink
* OpenLink
* (read)
* WriteLink

We want to hook them up, input of one is output of other, and vice-vera.
Assume that `LookatLink` is a god-given right, and always valid. Lookup
reveals that `TerminalStream` has two operations: the open operation
uses `OpenLink` having no arguments and promises to deliver a steam of
`ItemNode`s. So it is a pure source. The write operation uses `WriteLink`
which takes one argument, an `ItemNode` and has no outputs. Its a pure sink.

Specifically, running
```
(cog-execute! (Lookup (Type 'TerminalStream)))
```
returns the following:
```
(Choice
  (Section
    (Item "the open terminal command")
    (ConnectorSeq
      (Connector (Sex "command") (Type "OpenLink"))
      (Connector (Sex "reply") (Type "TerminalStream"))))
  (Section
    (Item "the write stuff command")
    (ConnectorSeq
      (Connector (Sex "command") (Type "WriteLink"))
      (Connector (Sex "command") (Type "ItemNode")))))
```

Now we have a problem. How connects up to the open command, and
runs it? I could run it manually, of course, and then hook up
everything else, too, manually. See the demo.

But doing it manually is cheating: the agent needs to do this.
So, for starters, for Linkages to make sense, the agent must exhibit
a Section:
```
(cog-execute! (Lookup (Type 'MyFooAgent)))
```
which returns
```
  (Section
    (Item "piping agent")
    (ConnectorSeq
      (Connector (Sex "issuer") (Type "OpenLink"))
      (Connector (Sex "issuer") (Type "OpenLink"))))
```
Note this is constructed so that it can open two things. Since the only
other thin in the dictionary will be the terminal, the result, after
linkage, will be two open terminals.

Note that the sexuality is a bit odd: The linker can pair up "issuer"
to "command".  That seems to be fine for now. The result is two open
terminals, call them A and B. Both of them have one unpaired connector,
which looks like this:
```
  (Connector (Sex "reply") (Type "TerminalStream"))
```
Now what?

Well, here's what we want to happen, but its not clear how. We want to
somehow note that each open terminal has a write command, so we want to
hook each reply stream to each writer. But how?

Well, lookup is a god-given right, so we look at the agent again,
but we've exhausted the possibilities. We've pruned the expression list
to zero.

We lookup the terminal again, and notice it has two choices: "open" and
"write" The "open" connector sequence is useless, because there's
nothing for it to connect to. It can be pruned. The "write" connector
sequence looks plausible. Certainly, it has oe interesting connector:
```
   (Connector (Sex "command") (Type "ItemNode")))
```
which can be paired to the stream. For his, we have to allow sexual
pairing of command-reply which I guess is a lot like issuer-commond,
but I guess it impliictly says there's a stream here? vs. issuer is
a command run only once, replies are necessarily streams? Something
like that?

This now raises another problem: we've got two unconnected connectors
of the form
```
   (Connector (Sex "command") (Type "WriteLink"))
```
one of these on each terminal. So I guess we have to go back to the
Agent, and discover that it also has
```
  (Section
    (Item "piping agent")
    (ConnectorSeq
		(Connector (Sex "issuer") (Type "WriteLink"))
		(Connector (Sex "issuer") (Type "WriteLink"))))
```
Given these, all connectors can now be full paired, the linkage is
complete, and we created a fully linked parse.

Did we need to create a second instance of the agent to obtain this
Section? Sure, why not. At this time we have no machanism to ask
the existing agent if it can also issue this alternate Section. Hmmm.
This might eventually become a problem? For this demo, it seems not to
be.

We've successfully mated all of the connectors, so we are done with
**stage one**. There is only one linkage, so **stage two** is trivial.
Well returns to stage three shortly.

For now, there remains a minor issue: we've been sloppy withy the
connector mating.  Some of these seem like one-shot fires, some seem
like streams, its confusing. This needs to be tightened up.

Or maybe skip to stage three, and return to the connector mating issue
later. Lets see... Hmmm.

Stage three
-----------
In this stage, given the proprsed pairing, we create the actual pipes.
Two problems:
* How is the list of proposed pairings delivered to us?
* How do we create actual links?

If we do Stage One correctly, then no instances have been created yet.
They will be, only now, during stage three. Stage One nees to return
a list
```
(AndLink
	(List (CrossSection...) (CrossSection...))
	(List ... ...)
	...)
```
Recall that `CrossSections` are built from Sections by isolating one
particular connector: this is the connector that is to be mated. Yow!
The good news is that we already invented the concept of `CrossSections`
a few years ago. The bad news is they are a bit gnarly.

Next, we walk over the list of pairs in the `AndLink`, and for each
pair, create a link pf the appropriate type. What does this mean? How
is this to be done? Ugh. Once again, dive into the details.

Creating one link
-----------------
So, we now have to create  a link for
```
	; From the Agent CrossSection:
   (Connector (Sex "issuer") (Type "OpenLink"))
	; From the terminal CrossSection
   (Connector (Sex "command") (Type "OpenLink"))
```
We already know the sex mating is allowed, we discovered this in stage
one. We look at both cross sections, and discover that the body of the
cross for the terminal is empty, so we can just perform
```
   (cog-execute (Open (Type 'TerminalStream)))
```
Oh effing A. This returns an instance. Is it a god-given fact that Open
always returns instances? I guess so. Which means we need to record this
instance somewhere.

We do this for the other terminal instance. But how do we know that the
Agent can now be retired, because both commands ran? We got lucky, here,
because the `Open` takes no arguments. For the `Write` we cannot retire
so easily; we have to pass args which arrive on different connectors.
Ick.

... Hmmm

-----------------------------------
