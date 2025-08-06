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

Examples are in order.

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


### TODO
* ephemeral like open
* 
