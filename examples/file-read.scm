;
; file-read.scm -- file-reading demo
;
; Demo opening a file and streaming the contents into a processing
; pipeline.
;
(use-modules (opencog) (opencog exec) (opencog sensory))

; Before running this demo, copy `demo.txt` to the /tmp directory.
; This is a text file, it will be read and processed in this demo.
(copy-file "demo.txt" "/tmp/demo.txt")

; --------------------------------------------------------
; Basic demo: Open a file for reading, at a fixed absolute location
; in the filesystem. Sending the (Predicate "*-read-*") message to
; the (TextFile "file:///tmp/demo.txt") object will cause one line
; to be read from the file, and returned as a StringValue.

;; Memoize the TextFileNode in scheme. This does tow things:
;; 1) Avoids typing, below.
;; 2) Avoids hitting the AtomSpace repeatedly.
(define file-node (TextFile "file:///tmp/demo.txt"))

(cog-execute!
	(SetValue file-node (Predicate "*-open-*")
		(ConceptNode "This is my EOF marker")))

; Read one line of text from the file. Do this by sending the *-read-*
; message to the file object.
(cog-execute! (ValueOf file-node (Predicate "*-read-*")))

; Do it again and again. Keep doing it to EOF is reached.
(cog-execute! (ValueOf file-node (Predicate "*-read-*")))
(cog-execute! (ValueOf file-node (Predicate "*-read-*")))
(cog-execute! (ValueOf file-node (Predicate "*-read-*")))
(cog-execute! (ValueOf file-node (Predicate "*-read-*")))

; --------------------------------------------------------
; Convert the line-by-line reder into a streaming reader. Working with
; StreamValues simplifies processing pipelines. StreamValues will
; deliver more data with each reference, avoiding the need to loop
; over the *-read-* message.
;
; Each examination of the stream will return a line from the file,
; in sequential order.

(define txt-stream (ReadStream file-node))

; Lets rewind to the begining.
(cog-execute!
	(SetValue file-node (Predicate "*-open-*")
		(ConceptNode "Another EOF marker")))

; Repeated references to the stream will return single lines from
; the file.
txt-stream
txt-stream
txt-stream
txt-stream
txt-stream

; Eventually, this will return the EOF marker.

; --------------------------------------------------------
; Demo: Perform indirect streaming. The text-stream will be placed
; as a Value on some Atom, where it can be accessed and processed.
;
; Open the file again, placing the text stream at "some place".
(cog-set-value! (Concept "foo") (Predicate "some place")
	(cog-value
		(TextFile "file:///tmp/demo.txt")
		(Predicate "*-read-*")))

; A better, all-Atomese version of the above. When the SetValueLink
; is executed, it will attach the freshly opened reader stream at
; "some place".
(cog-execute!
	(SetValue (Concept "foo") (Predicate "some place")
		(ValueOf
			(TextFile "file:///tmp/demo.txt")
			(Predicate "*-read-*"))))

; The freshly-opened stream can be accessed by just fetching
; it from "some place", the location it is anchored at.
(define txt-stream-gen
	(ValueOf (Concept "foo") (Predicate "some place")))

; Like all Atomese, the ValueOf is just a declaration: it doesn't
; "do anything"; it just "sits there", passively. To actually access
; the text stream, it needs to be executed. Each execution advances
; the stream pointer, getting the next line in the file.
(cog-execute! txt-stream-gen)
(cog-execute! txt-stream-gen)
(cog-execute! txt-stream-gen)
(cog-execute! txt-stream-gen)
(cog-execute! txt-stream-gen)

; --------------------------------------------------------
; Demo: Perform processing on the stream. For each line of the
; input file, apply a rule to rewrite it into a different form.

; This rule just makes two copies of each input line, interleaving
; it with other text. It is built with the same ValueOf promise, as
; above.
(define rule-applier
	; When a filter is executed, it applies the first argument
	; to the second. The first argument here is a Rule, the
	; second is the text file stream.
	(Filter
		; A rewrite rule has three parts: a variable declaration,
		; a pattern to match, and the rewrite to apply. Here, the
		; pattern match is trivial: `(Variable "$x")` matches
		; everything, the entire body of the input, which will be
		; a line from the text file. More pecisely, and ItemNode
		; holding that line. The rewrite below is just some
		; silliness that makes two copies of the input.
		;
		; The LinkSignatureLink is a constructor: it creates either
		; a LinkValue or a Link of the specified type. In this demo,
		; just using a plain-old List (instead of the LinkSignature)
		; would have been OK. But if the Variable had been a Value,
		; then using the LinkSignature would have been required.
		(Rule
			(TypedVariable (Variable "$x") (Type 'ItemNode))
			(Variable "$x")
			(LinkSignature       ; Or use List here and skip next line.
				(Type 'LinkValue)
				(Variable "$x")
				(Item "yo the first\n")
				(Variable "$x")
				(Item "yo the second\n")
				(Item "====\n")))
		txt-stream-gen))

; The previous demo ran the input file to end-of-file; we need to
; restart at the beginning.
(cog-execute!
	(SetValue (Concept "foo") (Predicate "some place")
		(ValueOf
			(TextFile "file:///tmp/demo.txt")
			(Predicate "*-read-*"))))

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
; Demo: Perform processing on the stream. In this case, parse the
; input stream into token pairs. Use the LG "any" parser for this.

(use-modules (opencog nlp) (opencog nlp lg-parse))

; As above: rewind the stream to the beginning:
(cog-execute!
	(SetValue (Concept "foo") (Predicate "some place")
		(ValueOf
			(TextFile "file:///tmp/demo.txt")
			(Predicate "*-read-*"))))

; Gentle reminder of how to fetch this:
(define txt-stream-gen
	(ValueOf (Concept "foo") (Predicate "some place")))

; Parse the file contents, one line at a time. The "any" dict generates
; random word-pairs. The (Number 1) asks for only one parse per
; sentence.
(cog-execute! (LgParseBonds txt-stream-gen (LgDict "any") (Number 1)))
(cog-execute! (LgParseBonds txt-stream-gen (LgDict "any") (Number 1)))
(cog-execute! (LgParseBonds txt-stream-gen (LgDict "any") (Number 1)))

; --------------------------------------------------------
; The End! That's All, Folks!
