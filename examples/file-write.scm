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



;;;; --------------------------------------------------------
;;;; Demo: Perform indirect streaming. The file-stream will be placed
;;;; as a Value on some Atom, where it can be accessed and processed.
;;;;
;;;; Open the file, get the stream, and place it somewhere.
;;;(cog-set-value! (Concept "foo") (Predicate "some place")
;;;	(cog-execute! (TextFileNode "file:///tmp/demo.txt")))
;;;
;;;; A better, all-Atomese version of the above. Note that the SetValueLink
;;;; will execute the TextFileNode, grab whatever it gets from that exec,
;;;; and then places it at the indicated location.
;;;(cog-execute!
;;;	(SetValue (Concept "foo") (Predicate "some place")
;;;		(File "file:///tmp/demo.txt")))
;;;
;;;; Define an executable node that will feed the stream of text.
;;;(define txt-stream-gen
;;;	(ValueOf (Concept "foo") (Predicate "some place")))
;;;
;;;; Access the file contents. Each time this is executed, it gets the
;;;; next line in the file.
;;;(cog-execute! txt-stream-gen)
;;;(cog-execute! txt-stream-gen)
;;;(cog-execute! txt-stream-gen)
;;;(cog-execute! txt-stream-gen)
;;;(cog-execute! txt-stream-gen)
;;;
;;;; --------------------------------------------------------
;;;; Demo: Perform processing on the stream. In this case, parse the
;;;; input stream into token pairs. Use the LG "any" parser for this.
;;;
;;;(use-modules (opencog nlp) (opencog nlp lg-parse))
;;;
;;;; As above: rewind the stream to the begining:
;;;(cog-execute!
;;;	(SetValue (Concept "foo") (Predicate "some place")
;;;		(File "file:///tmp/demo.txt")))
;;;
;;;; Parse the file contents, one line at a time. The "any" dict generates
;;;; random word-pairs. The (Number 1) asks for only one parse per
;;;; sentence.
;;;(cog-execute! (LgParseBonds txt-stream-gen (Number 1) (LgDict "any")))
;;;(cog-execute! (LgParseBonds txt-stream-gen (Number 1) (LgDict "any")))
;;;(cog-execute! (LgParseBonds txt-stream-gen (Number 1) (LgDict "any")))
