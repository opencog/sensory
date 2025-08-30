Objects and Streams
-------------------
The previous entry [Design Nodes I](./DesignNotes-I.md) describes a
"parametric polymorphism" object interface into Atomese, leveraging the
C++ `setValue()` and `getValue()` methods to perform dynamic dispatch
of messages. The idea there is that any Node deriving from `ObejctNode`
will support some collection of messages that can be sent to it, and
then behave appropriately to those messages. This seems like the
appropriate interface for anything that
 * Will be around for a long length of time (and should thus be an Atom)
 * Maintains internal state (and thus should be around a long length of
   time)
 * If not that, then anything that shadows something that maintains state
   (e.g. a sensor into the external world.)

This design notion is different that what is implemented in version 0,
and the question arises: how should the interfaces in v0 be modified to
match this new OO message-passing design?  Lets review the v0 API. It's
this:

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
dispatcher. Ouch. This seems awfully muddled.

The object API for the above would be quite different. Instead of a
`TextFileStream` and a `IRCChatStream`, there would be a `TextFileNode`
and an `IRCChatNode`. The `IRCChatNode::getValue()` method would then
take a `(Predicate "*-open-*")` message, and return a `StringStream`
or whatever.

There does not seem to be any easy way to migrate from the one to the
other, and so it looks like a total rewrite will be necessary. Ugh.

August 2025 update: Total re-write begun. Both `OpenLink` and
`WriteLink` are gone. The `OpenLink` seems un-needed, because the
`(Predicate "*-open-*)` message does the trick. Its more generic.

The `WriteLink` is gone, but raises a new problem. The v0 `WriteLink`
had a built-in infinite loop that would suck a stream dry, pushing it
out somwhere. The current default object api doesn't expose the notion
of pulling from a stream. But having this kind of auto-run info loop
driver feels like a good idea.  How should it be implemented?

Well, lets re-examine the v0 implementation. It worked like this:
 * All streams capable of being written to inherit from OutputStream.
   OutputStream ha a method ::write_out() that has the infinite loop.
 * WriteLink wraps two Atoms. execute! on the first returns an
   OutputStream. ::write_out()  is called, passing the second:
```
WriteLink::execute() {
    OutStreamPtr osp = _outgoing[0]->execute();
    osp->write_out(_outgoing[1]);
}

OutputStream::write_out(Handle from)
{
    source = from->execute();
    while (true) {
       val = source->value();
       write_one(val)
    }
}
```

The object version of this has to be similar, I guess: a Link that
takes two ObjectNodes, calling `*-read-*` on one, to get a Value,
then giving that to the other via `*-write-*`. So, pulls from one,
pushes to the other.

What should this be called? `PullerPusherLink` or uhh, `MotorLink`.
Sounds sexy. Is this needed? The current `xterm-bridge.scm` already
does this in pure Atomese:  It defines one-shot copiers:
```
(define copy-one-b-to-a
   (SetValue axterm (Predicate "*-write-*")
      (ValueOf bxterm (Predicate "*-read-*"))))
```
which copies exactly one value, and a convoluted tail call:
```
(Define
   (DefinedProcedure "b-to-a-tail")
   (PureExec copy-one-b-to-a (DefinedProcedure "b-to-a-tail")))
```
So the actual copying is easy, and the user needs no help hooking
up pipelines as complicated as desired. The tail call is ugly.
It's valid, but its ugly. What are the alternatives?
```
(LooperLink (cog-atomspace) copy-one-b-to-a)
```
which runs the loop until an exception is thrown. Just use
`ExecuteThreaded` to put it in it's own thread. Like `PureExec`
it discards the execution results. Well, or it could queue them
up, and is you don't want them, wrap with, uhh, `DevNullLink`
which passses execution to the wrapped links and discards the
return values. But that's confusing if the return is a `QueueValue`
which is not closed, and can thus grow and grow... Does it need to
actively discard? Maybe...
