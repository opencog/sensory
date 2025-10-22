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
(cog-execute!
	(SetValue file-node (Predicate "*-open-*") (Type 'StringValue)))

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
			(TypedVariable (Variable "$x") (Type 'StringValue))
			(Variable "$x")
			(LgParseBonds (Variable "$x") (LgDict "en") (Number 4)))
		txt-stream))

; Sniff test. Does it work?
; (cog-execute! parser)

; Create the a(cog-execute! bonder)nchor for the parsed text.
(cog-execute!
	(SetValue (Anchor "parse pipe") (Predicate "parsed text")
		(DontExec parser)))

; Anchor reference
(define parse-stream
	(ValueOf (Anchor "parse pipe") (Predicate "parsed text")))

; Sniff test. Does it work?
; (cog-execute! parse-stream)

; --------------------------------------------------------
; A rule that unto 

(define linker
	(Filter
		(Rule
			(TypedVariable (Glob "$linkages") (Type 'LinkValue))
			(LinkSignature (Type 'LinkValue)
				(Glob "$linkages"))
			(Glob "$linakges"))
		parse-stream))

; Sniff test. Does it work?
; (cog-execute! linker)

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
		parse-stream))

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
