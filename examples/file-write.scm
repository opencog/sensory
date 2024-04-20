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

; Get a fresh handle to the input stream:
(cog-execute!
	(SetValue
		(Concept "source") (Predicate "key")
		(TextFileNode "file:///tmp/demo.txt")))

; And now write again:
(cog-execute! writer)

; Verify that a second copy was written: `cat /tmp/foobar.txt`

; The End! That's All, Folks!
