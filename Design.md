System Design
=============
Architecture, Design and Implementation of the Sensory Action-Perception
system.

Parts:
* Description of the current API
* TODO List
* Diary/Notes of the thought process that went into the current design.

See the [examples](examples) directory for actual demos that should
actually work.

Diary
-----
A table of contents of the design diary. Many of these are early and
incomplete ideas about how things could work. Others are fundamental
design choices governing the system.

* [Notes Part A](DesignNotes-A.md) -- Early ideas for the design of
  the ReadLink & WriteLink. (early April 2024)
* [Notes Part B](DesignNotes-B.md) -- Basic idea of Action/Perception
  and basic idea of a DTD (Document Type Definition) or IDL (Interface
  Description Lanuage) for IRC chat.  First description of using Link
  Grammar Sections for the DTD/IDL.
  (mid-April 2024)
* [Notes Part C](DesignNotes-C.md) -- Sketch how to create a netlist
  or hookup diagram. The hookup requires assembling together a
  collection of disjuncts into a closed network.
  (late-April 2024)
* [Notes Part D](DesignNotes-D.md) -- Detail how to implement the
  hooker-upper.
  (late-April 2024)
* [Notes Part E](DesignNotes-E.md) -- Pipe and matrix ruminations.
  (early May 2024)

Current Implementation
----------------------
Documentation for the implementation internals. Lowlevel stuff. See the
[examples](examples) directory for demos on how this stuff should be
*used*. By contrast, the below explains how it *works*, which most
first-time users will *not* be interested in.

### OpenLink and WriteLink
There are two basic Atoms: `OpenLink` and `WriteLink`. The `OpenLink`
command is used to create a new I/O stream, and `WriteLink` is used to
write to the stream. Philosophically, `OpenLink` is a command that says
"go to a new place/location in the environment/exterior world, and open
a sensory stream at that location."  By contrast, `WriteLink` says "stay
at the current location, but spew text there." Write does not create a
new stream.

The `OpenLink` API is
```
	(OpenLink
		(TypeNode 'FoobarStream) ; e.g. TextFileStream or IRChatStream
		(SensoryNode "url://of/some/sort")) ; e.g file:// or irc://
```
The `TypeNode` must indicate a valid Atomese type that can be created.
The `SensoryNode` is just some text passed into the stream constructor.

Currently supported are `TextFileStream` and `IRChatStream`

The `WriteLink` API is
```
 (WriteLink
    (TextStream)   ; iterator to write to.
    (StringValue)) ; strings to write
```
Since links can't actually store Values, the above have to be executable
things that return the desired streams.

How does the C++ perform the write?
```
class OutputStream
	: public LinkStreamValue
{
	virtual ValuePtr write_out(const Handle&) = 0;
};

class TextFileStream : public OutputStream
{
private:
	FILE* _fh;

	ValuePtr write_out(const Handle& cref)
   {
		ValuePtr content =  cref->execute();
		StringValuePtr sv = StringValueCast(content);
		fprintf(_fh, "%s", sv->get_value());
		return sv;
	}
};

ValuePtr WriteLink::execute()
{
	TextStreamPtr = TextStreamCast(_outgoing[0]->execute());
	return ->write_out(_outgoing[1]);
}
```
That's it.

The `TextFileNode::execute()` creates new iterator,
and opens it for reading and writing.

TODO List
---------
* The text API should probably be changed to use StringValue instead
  of ItemNode. But this breaks LG parser, for now. FIXME
* The text file reader should probably throw on end-of-file, instead
  of returning empty content. This would allow intermediate processors
  to have an easier time of it.
* The concurrent queue should be changed to finite-size buffer and
  discard old data. Goal is to avoid unbounded-size chat buffers.
* The NICK command gets a badly-parsed response if the nick is in use.
* Need some kind of modal-open, so that the IRC nick is not hard-coded
  into the URL, but is rather specified during the open. So, open
  parameters. The Section concept fixes this, but remains unimplemented.
* Implement DontExecLink so when executed, it unwraps itself.
  Right now, this is hacked around in LinkSignatureLink
* The `load_csv_table` in AtomSpace persist should be a sensory node.

--------------------------------
