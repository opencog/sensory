Design notes
============
Thought process that went into current design.

Current Implementation
----------------------
The WriteLink API is
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
  discard old data.


Early draft notes
-----------------
Earlier ideas that lead up to the above.
For what the sensory I/O interface could be like.

Needs to be generic, powerful, flexible, anticipate future IO
requirements, be simple to use, simple to understand.

### First ideas
Lets go. Lets start from nowhere.

```
(ExecutionOutput
	(FileNode "///")  ; DefinedSchema  GroundedPredicate PROCEDURE_NODE
	(ValueOf stuff))
```
- nodes store state info with node
- links can too, because outoging. Sort of ish.
- links have args know how to execute.
- Just be a FunctionLink instead of ExecutionOutput
  Or ExecutableLink EXECUTABLE_LINK
```
(FileWriteLink
	(Node "file:///place")  ; Any Node at all can hold a string.
	(ValueOf stuff))  ; stuff to write fetched from this.

(WriteLink
	(Node "url:///place") ; Dispatch, branch on url
	(ValueOf stuff))

(WriteLink
	(FileNode "file:///place")  ; FileNode knows how to do it.
	(ValueOf stuff))
```
But how? WriteLink needs to grab stuff, and then call special
method on FileNode. So this starts to look like BackingStore but
with more limited special-purpose API's.
```
(FileReadLink               ; Used to place read data somewhere.
	(Node "file:///place")
	(SetValue stuff))

(ReadLink
	(FileNode "file:///place"))

(ReadLink
	(IRCBotNode "irc:///place?config=stuff&more=stuff"))
```

So how does it work?  `(cog-execute! (ReadLink ...))` needs to return
a value holding an iterator holding a pointer into the stream. The
iterator is a StreamValue and each call to `update()` on that stream
advances the iterator.

Iterators depend on the actual IO device, and so there needs to be a
`virtual ValuePtr SensoryNode::get_read_iterator();` that is called by
`ValuePtr ReadLink::execute()` and returned to the caller. The specific
iterator needs to know which stream it is reading from and other crap
like that.

A sufficiently general iterator can turn around and call methods on the
SensoryNode, so that e.g. `GenericSensoryStream::update()` calls
`SensoryNode::advance_iterator()` which does the actual reading.

We want to leave open the possibility of one SensoryNode being able to
handle multiple iterators, so each instance needs to have a private
class instance that provides the actual i/o handles. So lets sketch this:
```
class TextStream
	: public LinkStreamValue
{
	virtual void update() const = 0;
};

// Renamed version of PhraseStream today
class TextFileStream : public TextStream
{
private:
	FILE* _fh;

	TextFileStream(std::string& file_to_open);

public:
	void update()
	{
		fread (_fh, ...);
	}
};

class SensoryNode
{
	virtual ValuePtr make_new_read_iterator() = 0;
};

class TextFileNode : public SensoryNode
{
	ValuePtr make_new_read_iterator()
	{
		return createTextTextFileStream(_name);
	}
}

ValuePtr ReadLink::execute()
{
	SensoryNodeCast(_outgoing[0])->make_new_read_iterator();
}
```

If `TextFileNode` wanted to refuse more than one read iterator, it could
do the refusal in `make_new_read_iterator()`.

This is all very nice, but a ReadLink does not seem to be needed.
Just tapping the value is enough.

Writing
-------
What about writing? Calling `WriteLink::execute()` could eager-eval
the args and then pass them to the SensoryNode, or it could avoid
that, doing lazy-eval.

### Write bad idea
First idea, but its broken. Above would look like this:
```
ValuePtr WriteLink::execute()
{
	return SensoryNodeCast(_outgoing[0])->write_stuff(_outgoing[1]);
}

class SensoryNode
{
	virtual ValuePtr write_stuff(const Handle&) = 0;
};

class TextFileNode : public SensoryNode
{
	ValuePtr write_stuff(const Handle& args)
   {
		// Beta reduce and execute and etc.
		ValuePtr vargs = args->execute();
		fwrite(_fh, ..., vargs);
		return vargs;
	}
}
```
The above is not stateful, which is maybe OK for a file, which we can
open, append, close, but is bad for chatbot, which must stay open.
When do we open? StorageNodes force user to call `cog-open` explicitly.
Can we do same here? Create an IONode class, which both StorageNode
and also SensoryNode inherit from? That way, they share `cog-open`?

The persist `cog-open` sets a global storage-node pointer. It's too much
of a hassle to try to keep that API and still make everything work. So
punt, and create `cog-io-open` etc. instead.

Placing open/close on the node de facto forces one iterator per node,
because only one iterator per open/close. Unless we set it up so that
ech call to open() returns a new iterator. How can I write to iterator?

### Write better idea
If we use iterators for the output... then the WriteLink API needs
to be
```
 (WriteLink
    (TextFileNode "file:///tmp/foo")
    (Value stream of things to write)
    (Iterator to write to))
```

Where did the iterator come from?
Maybe from `(cog-io-open (TextFileNode "file:///tmp/foo"))` ?

Is the Iterator an Atom or a Stream? Easier if its an Atom. But then
the usual globally-unique atom issues bite, which are inappropriate for
iterators. Which are effectively private, like file handles. So seems
that iterators must be Values.

But having `(cog-io-open ...)` is icky. Can we do something flow-like
instead?

Sure. Just use the stream returned by `(cog-execute! (TextFileNode ...))`

### Write final idea
A `ReadLink` is not needed.

If we use iterators for the output... then the WriteLink API needs
to be
```
 (WriteLink
    (TextStream)   ; iterator to write to.
    (StringValue)) ; strings to write
```
Since links can't actually store Values, the above have to be executable
things that return the desired streams.

How to write?
```
class OutputStream
	: public LinkStreamValue
{
	virtual ValuePtr write_out(const Handle&) = 0;
};

// Renamed version of PhraseStream today
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

So then, `TextFileNode::execute()` creates new iterator,
and opens it for reading or writing, or maybe both. Make it
URL-dependent. `file:mode//path` where mode is read, write,
append, truncate, etc.

https://en.wikipedia.org/wiki/File_URI_scheme
file:/path
file:///path
file://./path   dot is localhost
file://host/path

Streaming issues
----------------
Test failures:
```
FilterValueTest guile -s ../tests/atoms/flow/filter-value-test.scm
FilterFloatTest guile -s ../tests/atoms/flow/filter-float-test.scm
```

Generic idea of how old atomspace code works is this:
* Let `(Gen)` be an Atom that gens new values each time its executed.
  e.g. `(Time)`
* For example `(define f (Time))` so then `(cog-execute! f)`
* A `FutureStream` converts this function to a streaming value.
  For example, `(define fs (FutureStream (Time)))` and then
  `(cog-value->list fs)`
* But streams cannot be stored in the AtomSpace, so `Promise` does this:
  For example, `(define p (Promise (TypeNode 'FutureStream) f))`
  so that `(cog-execute! p)` returns fs.
* Filtering in this scenario is easy: For example,
  `(define f (Filter (Rule...) (Time)))` fits into above scenario.

But this breaks for the current design because `TextFileNode` is
already a promise, and the above process is exactly backwards: we
want to apply the filtering to the output of the promise, and not
vice-versa.

So we need something like this:
```
(define f (Filter (Rule...) (Promise ...)))
```
so that `(cog-execute! f)` will create a stream from the promise,
and apply rule to it. But the creation of the stream can only be
performed once; repeated calls to execute the promise are not allowed.

Well, but that's easy: `ValueOf` behaves exctly like a promise:
```
(define f (Filter (Rule...) (ValueOf ...)))
(define p (Promise (TypeNode 'FutureStream) f))
```
and now, `p` can be used as a streaming source.


Constructors
------------
The current `TextFileNode` class is almost useless. Surely we can do
better. How about this, instead:
```
	(OpenLink
		(TypeNode 'TextFileStream)
		(SensoryNode "file:///file/system/path"))
```

This would return an instance of the given stream. It's a generic stream
constructor. Hopefully, the current ValueFactory can handle this.


--------------------------------
