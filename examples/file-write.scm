;
; file-write.scm -- file-writing demo
;
; Demo of writing Atoms and Values to a text file. This includes
; streaming to a file.
;
(use-modules (opencog) (opencog exec) (opencog sensory))

; --------------------------------------------------------
; Basic demo: Open a file for writing, and place some text into it.
; Open the file for writing; create it if it does not exist.
; Position cursor at the end of the file (for appending).
; The returned stream should be thought of as a cursor into the file.
(define txt-stream
	(cog-execute! (TextFileNode "file:///tmp/foobar.txt")))

; Perform `ls -la /tmp/foo*` and you should zee a file of zero length.

; A WriteLink consists of two parts: where to write, and what
; to write. Since the write cursor is a Value, not an Atom, we
; cannot specifiy it directly. Thus, we place it at an anchor
; point.
(cog-set-value!
	(Concept "file anchor") (Predicate "some key") txt-stream)

; Create a WriteLink
(define writer
	(WriteLink
		(ValueOf (Concept "file anchor") (Predicate "some key"))
		(Concept "stuff to write to the file\n")))

; Creating above does not write anything to the file.
; Verify this with another `ls -la /tmp/foo*`

; Write stuff to the file.
(cog-execute! writer)

; Verify that it was written: `cat /tmp/foobar.txt`
(cog-execute! writer)

; Do it a few more times.
(cog-execute! writer)
(cog-execute! writer)
(cog-execute! writer)

; --------------------------------------------------------
; Demo: Perform indirect streaming. The text to write will be placed as
; a StringValue at some location, and writing will be done from there.

(cog-set-value!
	(Concept "source") (Predicate "key")
	(StringValue
		"some text\n"
		"without a newline"
		"after it\n"
		"Goodbye!\n"))

; Redefine the writer.
(define writer
	(WriteLink
		(ValueOf (Concept "file anchor") (Predicate "some key"))
		(ValueOf (Concept "source") (Predicate "key"))))

; Write it out.
(cog-execute! writer)
(cog-execute! writer)
(cog-execute! writer)

; Verify that it was written: `cat /tmp/foobar.txt`

; --------------------------------------------------------
; Demo: Combine the reader and the writer to perform a file copy.
; Just as in the `file-read.scm` demo, the demo file should be copied
; to the temp directory, first: `cp demo.txt /tmp`

; Instead of using the custom scheme API `cog-set-value`, use the
; generic cog-execute! instead, and the SetValueLink to wire things
; into place.
(cog-execute!
	(SetValue
		(Concept "source") (Predicate "key")
		(TextFileNode "file:///tmp/demo.txt")))

; Running the writer will enter an infinite loop, pulling one line
; at a time from the input file, and writing it to the output file.
; The loop exits when end-of-file is reached.
(cog-execute! writer)

; Verify that it was written: `cat /tmp/foobar.txt`

; Try it again. Note that nothing will happen, because the input
; file iterator is now sitting at the end-of-file. Verify that there
; were no changes: `cat /tmp/foobar.txt`
(cog-execute! writer)

; Get a fresh handle to the input stream. Each call
; to SetValue will create a new file handle.
(cog-execute!
	(SetValue
		(Concept "source") (Predicate "key")
		(TextFileNode "file:///tmp/demo.txt")))

; And now write again:
(cog-execute! writer)

; Verify that a second copy was written: `cat /tmp/foobar.txt`

; --------------------------------------------------------
; Demo: Combine the reader and the writer to read, perform some
; processing, and then write the processed data out.

(cog-execute!
	(SetValue
		(Concept "source") (Predicate "key")
		(Filter
			(Rule
				(TypedVariable (Variable "$x") (Type 'ItemNode))
				(Variable "$x")
				;(LinkSignature
				;	(Type 'LinkValue)
(List
					(Variable "$x")
					(Item "yo the first\n")
					(Variable "$x")
					(Item "yo the second\n")
					(Item "====\n")))
			(TextFileNode "file:///tmp/demo.txt"))))

; --------------------------------------------------------

; For this demo, we will be reading the same demo file several times.
; For each part of the demo, we want to start at the begining. Make
; this easy be defining a rewind function, now.
(define make-new-input-iterator
	(SetValue
		(Concept "source") (Predicate "raw file input key")
		(TextFileNode "file:///tmp/demo.txt")))

; Create a new file iterator for reading the demo text file.
(cog-execute! make-new-input-iterator)

; The ValueOf Atom is a kind of promise about the future: when
; it is executed, it will return the Value, whatever it is, at
; that time (at the time when the executiion is done).
(define data-promise
	(ValueOf (Concept "source") (Predicate "raw file input key")))

; Verify that the file contents are being read as expected.
; i.e. that the promise is being kept: each new execution returns
; one line of text from the input file.
(cog-execute! data-promise)

; Repeat until end-of-file
(cog-execute! data-promise)
(cog-execute! data-promise)
(cog-execute! data-promise)
(cog-execute! data-promise)
(cog-execute! data-promise)
(cog-execute! data-promise)

; Define a rule that will take each line of input text, and
; process it in some way. In this case, it will just make
; two copies of each line.
(define rule-applier
	(Filter
		(Rule
			(TypedVariable (Variable "$x") (Type 'ItemNode))
			(Variable "$x")
			(List
				(Variable "$x")
				(Item "yo the first\n")
				(Variable "$x")
				(Item "yo the second\n")
				(Item "====\n")))
		data-promise))

; Before running the above, reset the input file to the begining,
; again. We'd left it at end of file in the last part.
(cog-execute! make-new-input-iterator)

; Run the rule, once.
(cog-execute! rule-applier)

; Repeat it again, over and over, till the end-of-file.
(cog-execute! rule-applier)
(cog-execute! rule-applier)
(cog-execute! rule-applier)
(cog-execute! rule-applier)
(cog-execute! rule-applier)
(cog-execute! rule-applier)


; --------------------------------------------------------
; The End! That's All, Folks!
