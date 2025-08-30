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
which copies exactly one value, and a complicated-looking tail call:
```
(Define
   (DefinedProcedure "b-to-a-tail")
   (PureExec copy-one-b-to-a (DefinedProcedure "b-to-a-tail")))
```
So the actual copying is easy, and there seems to be enough
infrastructure in place to specify arbitrarily-complicated processing
pipelines.

What's missing is a sense of forced flow. Streams should "flow",
whereas tail-call looks like a force pump. Is there a way to "flow
naturally"?

The initial knee-jerk reaction is that the tail call is ugly. It's
valid, it works, it has OK performance. The tail call de-facto
expands into a C++ loop, under the covers; that's implemented in
`DefinedProcedureNode::execute()` and runs at more-or-less "native"
speed.

So it's ugly. What are the alternatives? Well, this:
```
   (LooperLink (cog-atomspace) copy-one-b-to-a)
```
which runs the loop until an exception is thrown. But is this better?
Its more modular: it wraps the entire loop, begining to end. Whereas
DefinedProceedure A could call B could call C which calls A, maybe.
If it calls A, then it recurses, and that forms athe loop, and it runs.
The executation path is not bounded, it can be splattered throught the
AtomSpace. It might not be clear that some given execution path results
in recursion.

XXX There muight be a bug in the implementation of `DefinedProceedure`
It's not obvious that a call A->B->C->A works "as expected".

So a `LooperLink` gives bounded loop execution. Is it worth implementing
right now? There's no obvious pressing need for it, so no.

Some details: `Looper` would run be default in the current thread; it
can be wrapped with `ExecuteThreaded` to put it in it's own thread.
Like `PureExec`, it seems it would need to discard execution results.
Well, that, or it could queue them up, and if the results are unwanted,
then, uh, lets see: wrap with, uhh, `DevNullLink` which would pass
execution to the wrapped links and discard the return values. Easy to
say, but that's confusing if the return is a `QueueValue` which is not
yet closed, and can thus grow and grow... Managing that is unclear.

At any rate, both the `Looper` and the `DefinedProcedure` are visible
pumps, visibly doing step-at-a-time processing. The v0 `OutputStream`
really was a stream: the loop was hidden in such a way that the stream
just appears to flow, as long as there's input data to be had.

Somehow, I want to take the copy-one-at-a-time expression, and turn
it into a stream. Declaratively so: Not by wrapping it in an explicit
`Looper` or `DefinedProcedure` but instead, a.. uhhh....
