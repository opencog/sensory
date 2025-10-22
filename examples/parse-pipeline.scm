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
	(SetValue (Concept "foo") (Predicate "some place")
		(DontExec (ValueOf file-node (Predicate "*-stream-*")))))

; Anchor reference
(define txt-stream-gen
	(ValueOf (Concept "foo") (Predicate "some place")))

; A rule to parse text using the (old) LG English parser.
(define parser
	(Filter
		(Rule
			(TypedVariable (Variable "$x") (Type 'StringValue))
			(Variable "$x")
			(LgParseBonds (Variable "$x") (LgDict "en") (Number 1)))
		txt-stream-gen))

(cog-execute! parser)

; --------------------------------------------------------
; The End! That's All, Folks!
