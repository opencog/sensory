;
; file-write.scm -- file-writing demo
;
; Demo of writing Atoms and Values to a text file. This includes
; streaming to a file.
;
(use-modules (opencog) (opencog exec) (opencog sensory))

; --------------------------------------------------------
; Basic demo: Open a file for writing, and place some text into it.
; Unlike the File reader, which returns a stream that can be read from,
; the writer is implemented by the object itself. Reader streams are
; opened automatically when the stream is created. Writers must be
; explicitly opened before writing.
;
; Upon opening, the file is created, if it does not exist.
; The cursor is positioned at the end of the file (for appending).
(define text-file (TextFile "file:///tmp/foobar.txt"))

(cog-execute!
	(SetValue text-file (Predicate "*-open-*") (Type 'StringValue)))

; Perform `ls -la /tmp/foo*` and you should see a file of zero length.

; Write some text to the file.
(cog-set-value! text-file
	(Predicate "*-write-*") (StringValue "Hello there!\n"))

(cog-set-value! (TextFile "file:///tmp/foobar.txt")
	(Predicate "*-write-*") (StringValue "How are you?\n"))

; --------------------------------------------------------
; Demo: Perform indirect streaming. The text to write will be placed as
; a StringValue at some location, and writing will be done from there.

(cog-set-value!
	(Concept "source") (Predicate "key")
	(StringValue
		"Some text\n"
		"Without a newline "
		"after it\n"
		"Goodbye!\n"))

; Define a writer, that, when executed, will write to the file.
(define writer
	(SetValue text-file (Predicate "*-write-*")
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
(copy-file "demo.txt" "/tmp/demo.txt")

; The writer was previously configured (just a few lines up above)
; to obtain text from (Concept "source") (Predicate "key"). Here,
; we replace that text with a file reader stream. Thus, when the
; writer is executed, it will suck text out of this stream, and
; write it to the output file.
(define input-text-file (TextFile "file:///tmp/demo.txt"))
(cog-execute!
	(SetValue
		(Concept "source") (Predicate "key")
		(ValueOf input-text-file (Predicate "*-stream-*"))))

; Open it before we do anything else.
(define please-be-kind-rewind
   (SetValue input-text-file (Predicate "*-open-*") (Type 'StringValue)))

(cog-execute! please-be-kind-rewind)

; Running the writer will enter a loop (infinite loop), pulling one
; line at a time from the input file, and writing it to the output file.
; The loop exits when end-of-file is reached.
(cog-execute! writer)

; Verify that it was written: `cat /tmp/foobar.txt`

; Try it again. Note that nothing will happen, because the input
; file iterator is now sitting at the end-of-file. Verify that there
; were no changes: `cat /tmp/foobar.txt`
(cog-execute! writer)

; Get a fresh input stream. Each execution of SetValue will create
; a new stream.
(cog-execute! please-be-kind-rewind)

; And now write again:
(cog-execute! writer)

; Verify that a second copy was written: `cat /tmp/foobar.txt`

; --------------------------------------------------------
; Demo: Combine the reader and the writer to read, perform some
; processing, and then write the processed data out.

; Create a new file iterator for reading the demo text file.
(cog-execute!
	(SetValue
		(Concept "source") (Predicate "raw file input key")
		(ValueOf input-text-file (Predicate "*-stream-*"))))

; Define a rule that will take each line of input text, and
; process it in some way. In this case, it will just make
; two copies of each line. This is the same filter as in the
; `file-read.scm` demo. Review that demo if the below looks
; strange.
;
; Note that the `ValueOfLink` below behaves as a kind of promise:
; when executed, it promises to get the current value. In this case,
; it will be the next line of the input file.
(define rule-applier
	(Filter
		(Rule
			(TypedVariable (Variable "$x") (Type 'StringValue))
			(Variable "$x")
			(LinkSignature
				(Type 'LinkValue)
				(Variable "$x")
				(Item "yo the first\n")
				(Variable "$x")
				(Item "yo the second\n")
				(Item "====\n")))
		(ValueOf (Concept "source") (Predicate "raw file input key"))))

; If we were to just `(cog-execute! rule-applier)`, we'd get exactly
; one line of the input file processed. This was already demoed in the
; `file-read.scm` demo. (If you're adventurous, you can try again here.
; Just remember to reset the input file iterator, when done.)
;
; We don't want to do just one line: we want to process the entire
; stream, until end-of-file. For that, create a promise to do that,
; when executed.
(define prom
	(Promise (TypeNode 'FutureStream)  rule-applier))

; Designate the promise as the source of data for the file writer.
(cog-execute!
	(SetValue (Concept "source") (Predicate "key") prom))

; Run the file-writer. This uses exactly the same definition as before.
; Be sure to `ls -la /tmp/foobar.txt` before and after running this,
; or just `cat` it, to see the output file contents change.
(cog-execute! writer)

; Do it again. Nothing happens, because the input file cursor is at
; the end of file.
(cog-execute! writer)

; Reset the input file cursor. (or pick a new input file.)
(cog-execute!
	(SetValue
		(Concept "source") (Predicate "raw file input key")
		(ValueOf
			(TextFile "file:///tmp/demo.txt")
			(Predicate "*-read-*"))))

; Now, the writer will run again.
(cog-execute! writer)

; If unclear about the promise, you can explore it several ways.
; One is to do this: (Be sure to reset the input file, first.)
;    (cog-value->list (cog-execute! prom))
; This will process one line at a time.
;
; Equivalently, use ValueOf to get the value:
;    (cog-value->list (cog-execute!
;        (ValueOf (Concept "source") (Predicate "key"))))
;
; The writer just invokes this second form in an infinite loop,
; until the end-of-file is reached, and then it halts.

; --------------------------------------------------------
; The End! That's All, Folks!
