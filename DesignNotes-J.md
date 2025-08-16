Objects and Streams
-------------------
The previous entry [Design Nodes I](./DesignNotes-I.md) describes a
polymorphic object interface into Atomese, leveraging the C++
`setValue()` and `getValue()` methods to perform dynamic dispatch
of messages. The idea there is that any Node deriving from `ObejctNode`
will support some collection of messages that can be sent to it, and
then behave appropriately to those messages. This seems like the
appropriate interface for anything that
 * Will be around for a long length of time (and should thus be an Atom)
 * Maintains internal state (and thus should be around a long length of
   time)
 * If not that, then anything that shadows something that maintains state
   (e.g. a sensor into the external world.)

This design notion is different that what is currently implemented here,
and the question arises: should the current interfaces here be modified
to match this new OO message-passing deisgn?  Lets review the current
API. It's this:

```
   (OpenLink
      (TypeNode 'FoobarStream) ; e.g. TextFileStream or IRChatStream
      (SensoryNode "url://of/some/sort")) ; e.g file:// or irc://
```
The `TypeNode` must indicate a valid Atomese Stream type.
The `SensoryNode` is some configuration text passed into the stream
constructor.

Here, the stream happens to also act like an object. This is not so bad,
except that the Streams need to be sent messages, i.e. must act like
objects in a more literal sense. This is solved by mandating that the
Stream have a C++ `::write_out()` method, which acts as the OO message
dispatcher. Ouch. This seems awefully muddled.

The object API for the above would be quite different. Instead of a
`TextFileStream` and a `IRCChatStream`, there would be a `TextFileNode`
and an `IRCChatNode`. The `IRCChatNode::getValue()` method would then
take a `(Predicate "*-open-*")` message, and return a `StringStream`
or whatever.

There does not seem to be any easy way to migrate from the one to the
other interfaces, and so it looks like a total rewrite will be
neccessary. Ugh.
