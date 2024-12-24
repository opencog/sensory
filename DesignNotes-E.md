Design Notes Part E -- Matrix API and Pipe API.
==============================================
***Early May 2024***

The pure-atomese parser seems to be working well, approximately
emulating the old matrix-based pipe, but maybe 3x faster. So that's
good. Raises the questions:

* Some of the base config from matrix API is missing, like, where to
  write counts. Can we replace the matrix API, now? How?
* Some of the atomese pipelines does things the matrix API used to do,
  like insert StoreValue in the chain. This is now done in the pipe.
  How do we specify pipe properties how to hook them up that isn't
  total ad hoc seat-of-the-pants stuff.

The `LookatLink` can return the API, describable as always. Some of the
API needs to only run once, to get a parameter, and not attach a stream.
So, if `WriteLink` attaches streams, maybe we can have `SendLink` to get
short non-streaming things? For example, to find the correct location to
store counts... or should there be a counting sensor, to which updated
count events are streamed! Ah hah!

Even so, most base objects will like very similar, so it seems eaier to
make the count target be configurable, even if we stream to it.

The update-count and then store flow is not obvious, because
update-count wants to stop the buck. Maybe it needs to send out an
"updated" message, which the store pipe listens for.

How do we specify connectors for all this, with minimal fuss, miminmal
verbosity? How are the connections formed? As in DesignNotes-C, we need
a connection compiler to perform the actual hookup.

Third alternative: half-way house, use the old matrix API (there's a lot
there) and swap out performance critical bits on an as-needed bases. Kind
of ugly, this last, but its easier!?

Overview
--------
There are several distinct questions this work is trying to address:

(1) How does one declare the function signature of a pipe in Atomese?
(2) How does one associate such signatures to specific instance of
    objects; that is, how does one declare an object in Atomese?
(3) How do we distinguish objects and instances?
(4) What is the simplest way of wiring up pipes, given their function
    signatures? Of course, this can always be done manually, which is
    requires no sophistication or abstraction, just brute-force coding
    willpower. The goal is to be able to say "please hok this to that"
    and have the wiring robot solder together that particular circuit.
(5) Aside from (4), what is the point of all of this? Why bother?

Some ruminations on (5):

It seems that Atomese is primarilty a declarative language, and not a
functional one. So parts one through 4 are hard, because they are
asking: how does one do functional programming within a declarative
language? The answer is "not very easily". The signature declarations
along are insanely complicated and tedious. Building up the wiring
system desired in part (4) will be a huge and complex task. And given
the required expenditure of effort, the question is "why bother?"

The answer is that I want to be able to build such processing pipelines
using whiz-bangy higher-level algorithms. Some hand-wavey intersection
of how sentences can be generated with link grammar, how effective trees
can be created and scored with as-moses, and how electronics-wiring
netlists can be generated with DL-NN. Exactly which and how, I don't
know, but the prerequisite is to be able to wire things up, and this
prerueq is not yet satisfied.

Asking any of these meta-layers to "describe" the circuit means that, at
lower layers, the language does have to a declarative language; if not
Atomese, then something like it.

The classical way of doing this is with a compiler: the program
description is compiled into an executable; so here, the description
netlist, and the off-the-shelf library of pipeline pieces with their
description are wired up.

Why not start with the existing matrix API, and use that as the
compilation target? Because the current matrix API does not provide an
interface definition (IDL, interface description language)
https://en.wikipedia.org/wiki/Interface_description_language

Why not use onf of the existing IDL's and target that? Might get more
mindshare that way? At the risk of fragility and inability to extend
and progress, because those IDL's have design philosophies that are not
aligned with the AtomSpace design goals and infrastructure. Effin A.
Why is everything so hard?

Lets say we did have the necessary IDL for Atomese pipes, and the
compiler to hook them up. Then what? What reasonable expectations can we
have of the higher-level systems?  Will they be able to do anything
noteworthy? Or is this a grand hope of "build it and they will come"?

Clearly this needs work and articulation.  Maybe elsewhere, not here.

Method calls
------------
The nice thing about KVP is that an atom+key can be interpreted as
object+method:
```
   (ValueOf (Anchor "some object") (Predicate "method name"))
```
but if we use this, things are not searchable. However, `Section`s don't
have the nice key-value thing without currying. Also, using `GetLink` to
pattern match is much slower than `ValueOf` ...
```
   (Section
       (Anchor "some object")
       (Predicate "method name")
       (ConnectorSeq ...))
```
... unless we make the method name the first part of the connector
sequence!

For example, the matrix `count-api` declares a default location for
counts:
```
   (Section
       (Anchor "default count API")
       (ConnectorSeq
           (Connector
               (SexNode "method name")
               (Predicate "counter location"))
           (Connector
               (SexNode "config parameter")
               (Predicate "*-TruthValueKey-*")))
```
Seems like that will work! Yayy!

So, how do we use this? The pipe pair counter needs config parameters:
* Name of dict `(LgDict "any")`
* Number of parses `(Number 6)`
* Where sentence counts are incremented: `(SentenceNode "ANY")`
* Where parse counts are incremented: `(ParseNode "ANY")`
* Where counts are stored: `(PredicateNode "*-TruthValueKey-*")`
* Initial count: `(Number 0 0 0)`
* Increment: `(Number 0 0 1)`
* Storage: `(RocksStorageNode "unknown until runtime")`

So how to actually make the plugging happen?


------------------
