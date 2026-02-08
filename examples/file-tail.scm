;
; file-tail.scm -- Tailing a changing file
;
; Demo that follows (tails) a file, placing the updates into a stream,
; and attached that stream to a processing pipeline.
;
; Please review file-read.scm first; this demo expands on some of the
; techniques there.
;
(use-modules (opencog) (opencog sensory))

; --------------------------------------------------------
; Open a file, and follw the changes being made to it.

(PipeLink (NameNode "tail file") (TextFile "file:///tmp/tail.txt"))

(Trigger
	(SetValue (NameNode "tail file") (Predicate "*-open-*")
		(Type 'StringValue)))

(cog-set-value! (NameNode "tail file") (Predicate "*-follow-*") (BoolValue #t))

; Read one line of text from the file. Do this by sending the *-read-*
; message to the file object.
(Trigger (ValueOf (NameNode "tail file") (Predicate "*-read-*")))

; Do it again and again. Keep doing it untile one of these blocks.
; When it blocks, it will wait until more text is appended to the file.
; Thus: `echo "gaba gaba hey" >> /tmp/tail.txt` will unstick it.
(Trigger (ValueOf (NameNode "tail file") (Predicate "*-read-*")))
(Trigger (ValueOf (NameNode "tail file") (Predicate "*-read-*")))
(Trigger (ValueOf (NameNode "tail file") (Predicate "*-read-*")))
(Trigger (ValueOf (NameNode "tail file") (Predicate "*-read-*")))

; --------------------------------------------------------
; The *-stream-* message will return a handle to a StreamValue
; that will return appended lines, one by one. Referencing the
; stream will dequeue them one at a time, again blocking when
; there's nothing there.

(Pipe
	(Name "tail stream")
	(StreamValueOf (NameNode "tail file") (Predicate "*-stream-*")))

; Repeated references to the stream will return single lines from
; the file. Remember to `echo First rule is ... >> /tmp/tail.txt`
(Trigger (Name "tail stream"))
(Trigger (Name "tail stream"))
(Trigger (Name "tail stream"))

; --------------------------------------------------------
; Demo: Perform processing on the stream. For each line of the
; input file, apply a rule to rewrite it into a different form.
;
; To make the stream processor "realistic", the text-dtream source
; needs to placed somewhere, where it can be found.  Notice a trick
; used here: the `DontExec` link shelters the `ValueOf`, and prevents
; it from being executed while the SetValue is running.
(Trigger
	(SetValue (Concept "foo") (Predicate "some place")
		(DontExec (ValueOf (NameNode "tail file") (Predicate "*-stream-*")))))

; Access to the stream.
(Pipe
	(Name "tail stream gen")
	(ValueOf (Concept "foo") (Predicate "some place")))

; This rule just makes two copies of each input line, interleaving
; it with other text. The `txt-stream-gen` is the source of input.
(Define
	(DefinedSchema "tail rule applier")
	(Filter
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
		(Name "tail stream gen")))

; Run the rule, once.
(Trigger (DefinedSchema "tail rule applier"))

; Repeat it again, over and over. This will block, unless the
; file is appended to. This will continue, until the text stream
; is sent the `*-close-*` message. If blocked, then the `*-close-*`
; has to be sent from another thread.
(Trigger (DefinedSchema "tail rule applier"))
(Trigger (DefinedSchema "tail rule applier"))
(Trigger (DefinedSchema "tail rule applier"))
(Trigger (DefinedSchema "tail rule applier"))
(Trigger (DefinedSchema "tail rule applier"))
(Trigger (DefinedSchema "tail rule applier"))

; --------------------------------------------------------
; The End! That's All, Folks!
