;
; parse-pipeline.scm -- pipelining demo
;
; An intended design point of the system is pipeline processing:
; Information that originates at a sensory node can be processed
; through a multi-step pipeline. This demo illustrates a file
; reader node, sourcing text, which is then passed through multiple
; pipeline stages. It is based on the read-file.scm demo, which
; demos the basic setup and framework.
;
; The Link Grammar parser is used for this demo, mostly because it
; generates fairly complicated data representations that are
; challenging to process.
;
(use-modules (opencog) (opencog exec) (opencog sensory))
(use-modules (opencog nlp) (opencog nlp lg-parse))

; Before running this demo, copy `demo.txt` to the /tmp directory.
; (copy-file "demo.txt" "/tmp/demo.txt")

; --------------------------------------------------------
; Set up the text source.

(define file-node (TextFile "file:///tmp/demo.txt"))
(define opener
	(SetValue file-node (Predicate "*-open-*") (Type 'StringValue)))
(cog-execute! opener)

; Create initial source anchor.
(cog-execute!
	(SetValue (Anchor "parse pipe") (Predicate "text source")
		(DontExec (ValueOf file-node (Predicate "*-stream-*")))))

; Anchor reference
(define txt-stream
	(ValueOf (Anchor "parse pipe") (Predicate "text source")))

; Sniff test: does it work?
; (cog-execute! txt-stream)

; --------------------------------------------------------
; A rule to parse text using the (old) LG English parser.
; Output is a pair of LinkValues: one containing the words
; in the sentence, and another with the links.
(define parser
	(Filter
		(Rule
			(TypedVariable (Variable "$x") (Type 'StringStream))
			; (Variable "$x") ; Use this for the untyped vardecl
			(Variable "$x")
			(LgParseBonds (Variable "$x") (LgDict "en") (Number 4)))
		txt-stream))

; Sniff test. Does it work?
; (cog-execute! parser)

; Create the anchor for the parsed text.
(cog-execute!
	(SetValue (Anchor "parse pipe") (Predicate "parsed text")
		(DontExec parser)))

; Anchor reference
(define parse-stream
	(ValueOf (Anchor "parse pipe") (Predicate "parsed text")))

; Sniff test. Does it work?
; (cog-execute! parse-stream)

; --------------------------------------------------------
; The parser above was configured to generate four linkages (parses)
; per sentance. These are bundled together in a single LinkValue.
; It is convenient to process them one at a time, rather than in one
; big gulp. The FlatStream provides this utility. It accepts as input,
; a stream where data items are bundled into lumps (with LinkValue),
; and then doles them out one by until the lump has been emptied. Then
; it gets the next lump from the input. In the below, four stream
; elements will be generated for each input sentence.

; The declaration is very simple: just promise to apply the flattener
; to the parse stream.
(define linker
	(PromiseLink (Type 'FlatStream)
	parse-stream))

; Just like the above. Create the anchor for the linkage text.
(cog-execute!
	(SetValue (Anchor "parse pipe") (Predicate "linkage stream")
		linker))

; Just as before: the Anchor reference.
(define linkage-stream
	(ValueOf (Anchor "parse pipe") (Predicate "linkage stream")))

; Sniff test. Does it work?
; (cog-execute! linkage-stream)

; --------------------------------------------------------
; A rule to extract just the bonds from the stream

(define bonder
	(Filter
		(Rule
			(VariableList
				(TypedVariable (Variable "$words") (Type 'LinkValue))
				(TypedVariable (Variable "$bonds") (Type 'LinkValue)))
			(LinkSignature (Type 'LinkValue)
				(Variable "$words")
				(Variable "$bonds"))
			(Variable "$bonds"))
		linkage-stream))

; Sniff test. Does it work?
; (cog-execute! bonder)

; Create the anchor for the parsed text.
(cog-execute!
	(SetValue (Anchor "parse pipe") (Predicate "parsed text")
		(DontExec parser)))

; Anchor reference
(define parse-stream
	(ValueOf (Anchor "parse pipe") (Predicate "parsed text")))

; --------------------------------------------------------
; The End! That's All, Folks!
