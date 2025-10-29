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
; For each sentence in the input text, the LG parser produces four
; linkages. This demo uses the `FlatStream` Value to flatten this
; list, and issue only one linkage at a time.
;
; Each linkage is a compund structure, having a list of words, and
; a list of bonds between words (word-pairs). A FilterLink is used
; to unwrap this structure, discard the words, and keep the bonds.
;
; --------
; Not implemented her (yet, but appears in main aatomspace demos):
; Count the number of word-pairs, and compute the MI between them,
; using nothing but a pure Atomese pipeline, continuing the below.
;
(use-modules (opencog) (opencog exec) (opencog sensory))
(use-modules (opencog lg))

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

; The declaration is simple: just promise to apply the flattener
; to the parse stream. Well, the flattener striped off one too
; many layers, so the LinkSignature puts one layer back.
(define linker
	(LinkSignature (Type 'LinkValue)
		(CollectionOfLink (Type 'FlatStream)
			(OrderedLink parse-stream))))

; Sniff test. Does it work?
; (cog-execute! linker)

; Just like the above. Create the anchor for the linkages.
(cog-execute!
	(SetValue (Anchor "parse pipe") (Predicate "linkage stream")
		linker))

; Just as before: the Anchor reference to the stream of linkages.
(define linkage-stream
	(ValueOf (Anchor "parse pipe") (Predicate "linkage stream")))

; Sniff test. Does it work?
; (cog-execute! linkage-stream)

; --------------------------------------------------------
; Each linkage has two parts: a word-list, and a list of linkages
; between the words. The word-list is reapeated each time, as the
; spelling-checker might select different alternatives for mis-spelled
; words. The tokenizer might split them differently. For this demo,
; we want to access only the bond list. The filer below will extract
; this.

(define bonder
	(Filter
		(Rule
			(VariableList
				(TypedVariable (Variable "$words") (Type 'LinkValue))
				(TypedVariable (Variable "$bonds") (Type 'LinkValue)))
			(LinkSignature (TypeInh 'LinkValue)
				(Variable "$words")
				(Variable "$bonds"))
			(Variable "$bonds"))
		linkage-stream))

; Sniff test. Does it work?
; (cog-execute! bonder)

; The above creates a double-wrapped list. Peel off one layer.
(define unwrapper
	(CollectionOfLink (Type 'FlatStream)
			(OrderedLink bonder)))

; Sniff test. Does it work?
; (cog-value->list (cog-execute! unwrapper))

; Create the anchor for the word bonds
(cog-execute!
	(SetValue (Anchor "parse pipe") (Predicate "word bonds")
		(DontExec unwrapper)))

; Anchor reference
(define bond-stream
	(ValueOf (Anchor "parse pipe") (Predicate "word bonds")))

; Sniff test. Does it work?
; (cog-execute! bond-stream)

; --------------------------------------------------------
; With the bonds between word-pairs available, one may now begin
; statistical analysis. A good place to begin is simply counting
; word-pairs.

(define counter
	(Filter
		(Rule
			(TypedVariable (Variable "$edge") (Type 'Edge))
			(Variable "$edge")
			(IncrementValueOn
				(Variable "$edge")
				(Predicate ":edge-count:")
				(Number 1)))
		bond-stream))

; Sniff test. Does it work?
; (cog-execute! counter)

; Create the anchor for the word bonds
(cog-execute!
	(SetValue (Anchor "parse pipe") (Predicate "counted bonds")
		(DontExec counter)))

; Anchor reference
(define counted-stream
	(ValueOf (Anchor "parse pipe") (Predicate "counted bonds")))

; Sniff test. Does it work?
; (cog-execute! counted-stream)


; --------------------------------------------------------
; The End! That's All, Folks!
