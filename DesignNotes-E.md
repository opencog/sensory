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
  How do we specifiy pipe properties how to hook them up that isn't
  total ad hoc seat-of-the-pants stuff.

The `LookatLink` can return the API, describable as always. Some of the
API needs to only run once, to get a paramter, and not attach a stream.
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
there) and swap out perfomance critical bits on an as-needed bases. Kind
of ugly, this last, but its easier!?

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
