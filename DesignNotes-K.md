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

