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


