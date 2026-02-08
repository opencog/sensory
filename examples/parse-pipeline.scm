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
(use-modules (opencog) (opencog sensory))
(use-modules (opencog lg))

; Before running this demo, copy `demo.txt` to the /tmp directory.
; (copy-file "demo.txt" "/tmp/demo.txt")

; --------------------------------------------------------
; Set up the text source.

(PipeLink (NameNode "file node") (TextFile "file:///tmp/demo.txt"))

(Define
	(DefinedSchema "opener")
	(SetValue (NameNode "file node") (Predicate "*-open-*")
		(Type 'StringValue)))

(Trigger (DefinedSchema "opener"))

; Create initial source anchor.
(Trigger
	(SetValue (Anchor "parse pipe") (Predicate "text source")
		(DontExec (ValueOf (NameNode "file node") (Predicate "*-stream-*")))))

; Anchor reference
(Pipe
	(Name "txt stream")
	(ValueOf (Anchor "parse pipe") (Predicate "text source")))

; Sniff test: does it work?
; (Trigger (Name "txt stream"))

; --------------------------------------------------------
; A rule to parse text using the (old) LG English parser.
; Output is a pair of LinkValues: one containing the words
; in the sentence, and another with the links.
(Define
	(DefinedSchema "parser")
	(Filter
		(Rule
			(TypedVariable (Variable "$x") (Type 'StringStream))
			; (Variable "$x") ; Use this for the untyped vardecl
			(Variable "$x")
			(LgParseBonds (Variable "$x") (LgDict "en") (Number 4)))
		(Name "txt stream")))

; Sniff test. Does it work?
; (Trigger (DefinedSchema "parser"))

; Create the anchor for the parsed text.
(Trigger
	(SetValue (Anchor "parse pipe") (Predicate "parsed text")
		(DontExec (DefinedSchema "parser"))))

; Anchor reference
(Pipe
	(Name "parse stream")
	(ValueOf (Anchor "parse pipe") (Predicate "parsed text")))

; Sniff test. Does it work?
; (Trigger (Name "parse stream"))

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
(Define
	(DefinedSchema "linker")
	(LinkSignature (Type 'LinkValue)
		(CollectionOfLink (Type 'FlatStream)
			(OrderedLink (Name "parse stream")))))

; Sniff test. Does it work?
; (Trigger (DefinedSchema "linker"))

; Just like the above. Create the anchor for the linkages.
(Trigger
	(SetValue (Anchor "parse pipe") (Predicate "linkage stream")
		(DefinedSchema "linker")))

; Just as before: the Anchor reference to the stream of linkages.
(Pipe
	(Name "linkage stream")
	(ValueOf (Anchor "parse pipe") (Predicate "linkage stream")))

; Sniff test. Does it work?
; (Trigger (Name "linkage stream"))

; --------------------------------------------------------
; Each linkage has two parts: a word-list, and a list of linkages
; between the words. The word-list is reapeated each time, as the
; spelling-checker might select different alternatives for mis-spelled
; words. The tokenizer might split them differently. For this demo,
; we want to access only the bond list. The filer below will extract
; this.

(Define
	(DefinedSchema "bonder")
	(Filter
		(Rule
			(VariableList
				(TypedVariable (Variable "$words") (Type 'LinkValue))
				(TypedVariable (Variable "$bonds") (Type 'LinkValue)))
			(LinkSignature (TypeInh 'LinkValue)
				(Variable "$words")
				(Variable "$bonds"))
			(Variable "$bonds"))
		(Name "linkage stream")))

; Sniff test. Does it work?
; (Trigger (DefinedSchema "bonder"))

; The above creates a double-wrapped list. Peel off one layer.
(Define
	(DefinedSchema "unwrapper")
	(CollectionOfLink (Type 'FlatStream)
		(OrderedLink (DefinedSchema "bonder"))))

; Sniff test. Does it work?
; (cog-value->list (Trigger (DefinedSchema "unwrapper")))

; Create the anchor for the word bonds
(Trigger
	(SetValue (Anchor "parse pipe") (Predicate "word bonds")
		(DontExec (DefinedSchema "unwrapper"))))

; Anchor reference
(Pipe
	(Name "bond stream")
	(ValueOf (Anchor "parse pipe") (Predicate "word bonds")))

; Sniff test. Does it work?
; (Trigger (Name "bond stream"))

; --------------------------------------------------------
; With the bonds between word-pairs available, one may now begin
; statistical analysis. A good place to begin is simply counting
; word-pairs.

(Define
	(DefinedSchema "counter")
	(Filter
		(Rule
			(TypedVariable (Variable "$edge") (Type 'Edge))
			(Variable "$edge")
			(IncrementValueOn
				(Variable "$edge")
				(Predicate ":edge-count:")
				(Number 1)))
		(Name "bond stream")))

; Sniff test. Does it work?
; (Trigger (DefinedSchema "counter"))

; Create the anchor for the word bonds
(Trigger
	(SetValue (Anchor "parse pipe") (Predicate "counted bonds")
		(DontExec (DefinedSchema "counter"))))

; Anchor reference
(Pipe
	(Name "counted stream")
	(ValueOf (Anchor "parse pipe") (Predicate "counted bonds")))

; Sniff test. Does it work?
; (Trigger (Name "counted stream"))

; --------------------------------------------------------
; The demo above always shows one step at a time. In practice,
; one will want to just run the pipeline until it completes,
; or perhaps run it forever, if the sream is endless. This can be
; accomplished with the DrainLink. It will loop forever, until
; it encounters a VoidValue, or a LinkValue of zero length, and
; the it will return.
;
; Like so:
(Trigger (Drain (Name "counted stream")))

; For an infinite stream, it can be run in it's own thread:
(Trigger (ExecuteThreaded (Drain (Name "counted stream"))))

; --------------------------------------------------------
; The End! That's All, Folks!
