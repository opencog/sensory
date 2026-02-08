;
; file-read.scm -- file-reading demo
;
; Demo opening a file and streaming the contents into a processing
; pipeline.
;
(use-modules (opencog) (opencog sensory))

; Before running this demo, copy `demo.txt` to the /tmp directory.
; This is a text file, it will be read and processed in this demo.
(copy-file "demo.txt" "/tmp/demo.txt")

; --------------------------------------------------------
; Basic demo: Open a file for reading, at a fixed absolute location
; in the filesystem. Sending the (Predicate "*-read-*") message to
; the (TextFile "file:///tmp/demo.txt") object will cause one line
; to be read from the file, and returned as a StringValue.

;; The NameNode is a handy handle for the TextFileNode, so that the
;; filename is not hard-coded further on in this demo.
(PipeLink (NameNode "file node") (TextFile "file:///tmp/demo.txt"))

;; Reading the file will return strings. But we have a choice of what
;; kind of strings we want: StringValue, or perhaps some kind of Node.
;; This time, we'll try out the StringValue.
(Trigger
	(SetValue (NameNode "file node") (Predicate "*-open-*")
		(Type 'StringValue)))

; Read one line of text from the file. Do this by sending the *-read-*
; message to the file object.
(Trigger (ValueOf (NameNode "file node") (Predicate "*-read-*")))

; Do it again and again. Keep doing it to EOF is reached.
(Trigger (ValueOf (NameNode "file node") (Predicate "*-read-*")))
(Trigger (ValueOf (NameNode "file node") (Predicate "*-read-*")))
(Trigger (ValueOf (NameNode "file node") (Predicate "*-read-*")))
(Trigger (ValueOf (NameNode "file node") (Predicate "*-read-*")))

; --------------------------------------------------------
; Convert the line-by-line reader into a streaming reader. Working with
; StreamValues simplifies processing pipelines. StreamValues will
; deliver more data with each reference, avoiding the need to loop
; over the *-read-* message.
;
; Each examination of the stream will return a line from the file,
; in sequential order.

; But first, let's rewind to the beginning. And change the return type,
; just for grins.
(Trigger
	(SetValue (NameNode "file node") (Predicate "*-open-*")
		(Type 'Concept)))

; The StreamValueOf automatically dequeues one item from the stream.
; The Pipe/Name sets up a named handle for convenient access.
(Pipe
	(Name "txt stream")
	(StreamValueOf (NameNode "file node") (Predicate "*-stream-*")))

; Repeated references to the stream will return single lines from
; the file.
(Trigger (Name "txt stream"))
(Trigger (Name "txt stream"))
(Trigger (Name "txt stream"))
(Trigger (Name "txt stream"))
(Trigger (Name "txt stream"))

; Eventually, this will return the EOF marker.

; --------------------------------------------------------
; The StreamValueOf used above accesses the stream directly. It can
; also be installed at some Atom, where it can be found by later
; pipeline stages.

; Again, let's rewind to the beginning.
(Trigger
	(SetValue (NameNode "file node") (Predicate "*-open-*")
		(Type 'Item)))

; Repeated references to the stream will return single lines from
; the file.
(Trigger (Name "txt stream"))
(Trigger (Name "txt stream"))
(Trigger (Name "txt stream"))

; --------------------------------------------------------
; Demo: Perform indirect streaming. The text-stream will be placed
; as a Value on some Atom, where it can be accessed and processed.
;
; Anchor the text stream at "some place", where it can be found.
; The DontExecLink allows the stream to be installed at "some place",
; without touching the stream during installation. Without it, the
; ValueOf would have been executed, and the file pointer would have
; been off by one.
(Trigger
	(SetValue (Concept "foo") (Predicate "some place")
		(DontExec (ValueOf (NameNode "file node") (Predicate "*-stream-*")))))

; The stream can be accessed by just fetching it from "some place",
; the location it is anchored at.
(Pipe
	(Name "txt stream gen")
	(ValueOf (Concept "foo") (Predicate "some place")))

; Like all Atomese, the ValueOf is just a declaration: it doesn't
; "do anything"; it just "sits there", passively. To actually access
; the text stream, it needs to be executed. Each execution advances
; the stream pointer, getting the next line in the file.
(Trigger (Name "txt stream gen"))
(Trigger (Name "txt stream gen"))
(Trigger (Name "txt stream gen"))
(Trigger (Name "txt stream gen"))
(Trigger (Name "txt stream gen"))

; --------------------------------------------------------
; Demo: Perform processing on the stream. For each line of the
; input file, apply a rule to rewrite it into a different form.

; This rule just makes two copies of each input line, interleaving
; it with other text. It is built with the same ValueOf promise, as
; above.
(Define
	(DefinedSchema "rule applier")
	; When a filter is executed, it applies the first argument
	; to the second. The first argument here is a Rule, the
	; second is the text file stream.
	(Filter
		; A rewrite rule has three parts: a variable declaration,
		; a pattern to match, and the rewrite to apply. Here, the
		; pattern match is trivial: `(Variable "$x")` matches
		; everything, the entire body of the input, which will be
		; a line from the text file. More pecisely, a StringValue
		; holding that line. The rewrite below is just some
		; silliness that makes two copies of the input.
		;
		; The LinkSignatureLink is a constructor: it creates either
		; a LinkValue or a Link of the specified type. If the stream
		; consisted of Atoms, then a plain-old List (instead of the
		; LinkSignature) would have been OK. But since the filtered
		; elements are Values, StringValues to be precise; they must
		; be wrapped with a LinkSignature.
		(Rule
			(TypedVariable (Variable "$x") (Type 'StringValue))
			(Variable "$x")
			(LinkSignature       ; Or use List here and skip next line.
				(Type 'LinkValue)
				(Variable "$x")
				(Item "yo the first\n")
				(Variable "$x")
				(Item "yo the second\n")
				(Item "====\n")))
		(Name "txt stream gen")))

; The previous demo ran the input file to end-of-file;
; restart at the beginning. Be sure to set the item type to
; StringValue, because that is what the filter expects.
(Trigger
	(SetValue (NameNode "file node") (Predicate "*-open-*")
		(Type 'StringValue)))

; Run the rule, once.
(Trigger (DefinedSchema "rule applier"))

; Repeat it again, over and over, till the end-of-file.
(Trigger (DefinedSchema "rule applier"))
(Trigger (DefinedSchema "rule applier"))
(Trigger (DefinedSchema "rule applier"))
(Trigger (DefinedSchema "rule applier"))
(Trigger (DefinedSchema "rule applier"))
(Trigger (DefinedSchema "rule applier"))

; --------------------------------------------------------
; Demo: Perform processing on the stream. In this case, parse the
; input stream into token pairs. Use the LG "any" parser for this.

(use-modules (opencog lg))

; As above: rewind the stream to the beginning:
(Trigger
	(SetValue (NameNode "file node") (Predicate "*-open-*")
		(Type 'StringValue)))

; Parse the file contents, one line at a time. The "any" dict generates
; random word-pairs. The (Number 1) asks for only one parse per
; sentence.
(Trigger (LgParseBonds (Name "txt stream gen") (LgDict "any") (Number 1)))
(Trigger (LgParseBonds (Name "txt stream gen") (LgDict "any") (Number 1)))
(Trigger (LgParseBonds (Name "txt stream gen") (LgDict "any") (Number 1)))

; --------------------------------------------------------
; The End! That's All, Folks!
