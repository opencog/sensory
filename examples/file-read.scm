;
; file-read.scm -- file-reading demo
;
; Demo opening a file and streaming the contents into a processing
; pipeline.
;
(use-modules (opencog) (opencog exec) (opencog sensory))

; Before running this demo, copy `demo.txt` to the /tmp directory.
; This is a text file, it will be read and processed in this demo.

; --------------------------------------------------------
; Basic demo: Open a file for reading, at a fixed absolute location
; in the filesystem. Executing the OpenLink will return a stream
; value that will deliver text strings, one line at a time.
(define txt-stream
	(cog-execute!
		(Open
			(Type 'TextFileStream)
			(SensoryNode "file:///tmp/demo.txt"))))

; Repeated references to the stream will return single lines from
; the file.
txt-stream
txt-stream
txt-stream
txt-stream
txt-stream

; Eventually, this will return an empty stream. This denotes end-of-file.

; --------------------------------------------------------
; Demo: Perform indirect streaming. The file-stream will be placed
; as a Value on some Atom, where it can be accessed and processed.
;
; Open the file, get the stream, and place it somewhere.
(cog-set-value! (Concept "foo") (Predicate "some place")
	(cog-execute!
		(Open (Type 'TextFileStream)
			(Sensory "file:///tmp/demo.txt"))))

; A better, all-Atomese version of the above. Note that the SetValueLink
; will execute the TextFileNode, grab whatever it gets from that exec,
; and then places it at the indicated location.
(cog-execute!
	(SetValue (Concept "foo") (Predicate "some place")
		(Open (Type 'TextFileStream)
			(Sensory "file:///tmp/demo.txt"))))

; Define an executable node that will feed the stream of text.
; The ValueOf Atom is a kind of promise about the future: when
; it is executed, it will return the Value, whatever it is, at
; that time (at the time when the executiion is done).
(define txt-stream-gen
	(ValueOf (Concept "foo") (Predicate "some place")))

; Access the file contents. Each time this is executed, it gets the
; next line in the file.
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
; restart at the begining.
(cog-execute!
	(SetValue (Concept "foo") (Predicate "some place")
		(Open (Type 'TextFileStream)
			(Sensory "file:///tmp/demo.txt"))))

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

; As above: rewind the stream to the begining:
(cog-execute!
	(SetValue (Concept "foo") (Predicate "some place")
		(Open (Type 'TextFileStream)
			(Sensory "file:///tmp/demo.txt"))))

; Parse the file contents, one line at a time. The "any" dict generates
; random word-pairs. The (Number 1) asks for only one parse per
; sentence.
(cog-execute! (LgParseBonds txt-stream-gen (LgDict "any") (Number 1)))
(cog-execute! (LgParseBonds txt-stream-gen (LgDict "any") (Number 1)))
(cog-execute! (LgParseBonds txt-stream-gen (LgDict "any") (Number 1)))

; --------------------------------------------------------
; The End! That's All, Folks!
