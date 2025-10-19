Watching and Following
----------------------
Working with a "real world" user of the file/directory interfaces.
Two needs arise: the ability to tail a file as it is being written by
some external writer, and the ability to watch directory contents, as
they are being manipulted by some external agent.

I gave these tasks to Clause, which seems to be coding up what I asked
for in a literal manner, but Claude is mindless and unthinking, and
like the wish-granting genie, isn't quite delivering what was wanted.

What's the issue? Well, there are several.

My original implementation provides this implementation for streaming
reads:
```
ValuePtr StreamNode::stream(void) const {  
   return createReadStream(get_handle());
}
```
In this implementation, `this` holds the handle to the thing being
streamed, and `ReadStream` plucks out one item at a time from the object
being read. This provides a reasonable implementation for those classes
that do not provide thier own implementation.

Classes that do provide thier own implementation typically return a
ContainerValue (QueueValue or UnisetValue). The ReadStream is a plain
LinkValue. So one minor question mark: Should it have been a
ContainerValue instead? Right now, that extra complexity seems
un-needed.

The issue (and this is my fault) is that I asked (allowed) Claude to add
a new method, `follow()`, to the Sensory API. As I write this, I realize
that it should just have been `stream()`. I guess this was a late-night
thinko. I will go correct this now.

The other is typical Claudeness. The tailing is done by using inotify.
But the inotify code is placed straight in, without modularization;
Claude is now inserting very nearly duplicate cut-n-paste into the
directory watcher. So this needs re-wrapping.

These are "minor" design issues, and are easily correctable at this
point. And yet, they reveal a "fluid design boundary": adding new
methods called `follow()` and `watch()` feels entirely natural. The
names reflect the functions they will provide, and the functions are
generic enouch that one imagines there will be other sensory nodes that
will want to provide these functions. These could have been accepted
into the C++ code. But they would cause down-stream issues. Right?

The current streaming design assumes everything flows through the
`stream()` method. This makes sense: after sending the `*-open-*`
message to an object, its reasonable to assume that what one gets back
is something that can stream data. This is what Values were originally
imagined to provide, when first implemented eight years ago: this is
what the `update()` method was envisioned for. But now, if we add the
`follow()` and `watch()` methods, there's an ambiguity: which stream do
you want to follow? How does the wiring diagram work? Ugh. The knee-jerk
response is that this all needs to be collapsed down to one: `stream()`.
Perhaps some future experiences will require something fancier, but the
what and how of this is opaque.

This fluidity of design choices comes very easily to Claude: it's happy
to do anything. Any pile of shit that vaguely matches what you asked
for, Claude will generate this. (Rest of philosophical ruminations moved
to `learn/learn-lang-diary/learn-lang-diary-part-ten-E.lyx`. Interesting
but out of place, here!?)

Watching and Following Redux
----------------------------
I am confusing myself. I think the new plan is this:
* Calling `*-stream-*` on a file (aka calling `SensoryNode::stream()`)
  should follow (tail) that file. The required flag needed to
  enable inotify should be set when `stream()` is called.

* For `FileSysNode`, there should be a "watch" command, much like the
  current support for "cd" or "ls". The `FileSysNode::write()` handles
  this. It creates a UnisetValue and launches a thread to fill that
  uniset. This is done for *each* watch, so that multiple watchers can
  run at the same time.

  The `FileSysNode::read()` returns that UnisetValue ... which is a
  ListValue, so attempts to access it will block until there are changes.

  Great? How to stop watching? Options: if the UnisetValue is closed,
  then the thread should be halted, and the blocked reader unblocks.
  But who has the handle to to the UnisetValue? The present flow design
  doesn't offer this control of ContainerValues. The current solution
  has to be that the stream doing the watch creates a new `FileSysNode`
  and then closes it when it is no longer interested in watching.

  A point of confusion: If two users specify `FileSysNode` with the
  same initial location, the AtomSpace will return the same object.
  This could be confusing for those who expected something else.
  Distinct  `FileSysNode` can be created by using `file:///bad-url`
  and then `cd`-ing to the desired location.
