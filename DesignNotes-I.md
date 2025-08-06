Sending Messages
----------------
There's a recurring pattern in the AtomSpace that has an inadequate
solution. In the below, I ponder the idea of "sending messages" as a
possbile solution.

The words "sending messages" hark back to a very old object-oriented
programming idea: there's an "object", you send it "messages".
Programmers who are used to C++/java say "calling methods" instead of
"sending messages".

Historically, AtomSpace Atoms were meant to be stateless, with a few
exceptions, such as `StateLink`. But then `StorageNode` showed up, and
it is stateful, more or less, in some ad hoc, ill-defined kind of way.
The `ProxyNode`s are stateful (they're a kind of `StorageNode`, so of
course.)

The problem with `StorageNode` and `ProxyNode` is that these need to be
configured.   There are config mechanisms, but they are ad hoc and
messy and generally unsatesfying. During the implementation, they felt
like a big hack.

Today, I was thinking that I would like to have a `CogServerNode`. But I
need to deal with config: what ports should it listen on? How can it be
started and stopped?

Well, there's one more stateful API with awkward or poorly defined
semantics: `GroundedSchemaNode`. It is meant to provide a way for
calling out of the AtomSpace. it's meant to be invoked as a function
call. But really, what seems to be happening is that the `GroundedSchema`
is just a handle into the external world, and executing it resembles the
sending of a message out to the external world (and, in this case,
getting an immediate reply.)  Perhaps we can use this as a model for
generic sending of messages.

### Objects are stateful
The base class for stateful nodes will be `ObjectNode`. Thus,
`atom_types.script` gets modified to something like this:
```
OBJECT_NODE <- NODE
GROUNDED_SCHEMA_NODE <- OBJECT_NODE
STORAGE_NODE <- OBJECT_NODE
COG_SERVER_NODE <- OBJECT_NODE
etc.
```
Messages can be sent to objects using `ExecutionLink` (no reply/void
reply) while output can be obtained with `ExecutionOutputLink`.

Perhaps it might be useful to have some C++ class to cache something
suitable for all `ObjectNode`s, but this is not yet clear. Perhaps a
list of valid messages they can receive? This is certainly desired with
the `Describe` functionality discussed and partly implemented here.

The name `ObjectNode` is consistent with traditional object-oriented
prgramming naming conventions: `ObjectNode`s are meant to be stateful
classes.

However, since the AtomSpace is designed to be effectively stateless,
the `OjectNode`s break the paradigm of saveability and restorability:
they can be saved only at the superficial level; internal state is
lost/erased when the power goes out. Worse, they are non-abelian, and
history dependent: the state of an `ObjectNode` depends on the messages
that have been received.

For the above reasons, the `ObjectNode` should really be considered to
be something that "lives out there", in the external world. Even if it
is purely local, the non-recoverability of state implies "something
happened out there, that we cannot control".

More directly, the `ObjectNode` can be considered to be ***the***
sensorimotor interface object: on the far side of the `ObjectNode` is
"the universe" and we can send messages "out there" and "receive stuff"
in response, but it is a deeply dividing boundary between internal and
external.

Examples are in order.

### Examples -- GroundedFunctionNode
There is no change at all to `GroundedSchemaNode`, etc. and they work
exactly as before. The only difference is the type hierarchy.

### Example -- Cogserver Config
The web port can be set on the cogserver by declaring
```
(Execution
   (CogServer "this is a singleton instance")
   (ListLink
      (Predicate "web-port")
      (Number 18001)))
```
The configuration does not take effect until the `ExecutionLink` is
executed. The message is the `ListLink`, but the message name is
`"web-port"` -- a string. Fits with common programming convention.

The cogserver can be started as
```
(Execution
   (CogServer "this is a singleton instance")
   (ListLink
      (Predicate "start")))
```

### Example -- ProxyNode config
The current ProxyNodes are configured as follows:
```
 (ProxyParameters
     (ReadThruProxy "read-thru load balancer")
     (List
        (RocksStorageNode "rocks:///tmp/foo.rdb")
        (RocksStorageNode "rocks:///tmp/bar.rdb")
        (RocksStorageNode "rocks:///tmp/fizz-buzz.rdb"))))
```
The current implementation has the `ReadThruProxy` reach back into it's
incoming set, find the `ProxyParamweters` link, then descend back down.
This happens upon the `cog-open` of the proxy.  The proposal is to
replace this by
```
 (Execution
     (ReadThruProxy "read-thru load balancer")
     (List
        (Predicate "mirrors")
        (RocksStorageNode "rocks:///tmp/foo.rdb")
        (RocksStorageNode "rocks:///tmp/bar.rdb")
        (RocksStorageNode "rocks:///tmp/fizz-buzz.rdb"))))
```

The `(Predicate "mirrors")` is a string message name; it allows for
other strings to be handled, in the future.

### Example: open and close
The `cog-open` scheme call acting on a StorageNode can be replaced by
```
(Execution
   (RocksStorageNode "rocks:///tmp/foobar.rdb")
   (List
      (Predicate "open")))
```
The open does not happen, until the above is actually executed.


### Problematic constructions
It is then tempting to handle all the other `StorageNode` ops in this
same way. Thus:
```
(Execution
   (RocksStorageNode "rocks:///tmp/foobar.rdb")
   (List
      (Predicate "store-atom")
      (Concept "foo")))
```
The problem here is that this pollutes the AtomSpace with lots of extra
gunk. We really don't want to do this. What alternatives do we have?


### TODO
* ephemeral like open
* pi calculus https://en.wikipedia.org/wiki/%CE%A0-calculus
