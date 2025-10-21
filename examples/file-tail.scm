;
; file-tail.scm -- Tailing a changing file
;
; Demo that follows (tails) a file, placing the updates into a stream,
; and attached that stream to a processing pipeline.
;
; Please review file-read.scm first; this demo expands on some of the
; techniques there.
;
(use-modules (opencog) (opencog exec) (opencog sensory))

; --------------------------------------------------------
; Open a file, and follw the changes being made to it.

(define file-node (TextFile "file:///tmp/tail.txt"))

(cog-execute!
	(SetValue file-node (Predicate "*-open-*") (Type 'StringValue)))

(cog-set-value!  file-node (Predicate "*-follow-*") (BoolValue #t))

; Read one line of text from the file. Do this by sending the *-read-*
; message to the file object.
(cog-execute! (ValueOf file-node (Predicate "*-read-*")))

; Do it again and again. Keep doing it untile one of these blocks.
; When it blocks, it will wait until more text is appended to the file.
; Thus: `echo "gaba gaba hey" >> /tmp/tail.txt` will unstick it.
(cog-execute! (ValueOf file-node (Predicate "*-read-*")))
(cog-execute! (ValueOf file-node (Predicate "*-read-*")))
(cog-execute! (ValueOf file-node (Predicate "*-read-*")))
(cog-execute! (ValueOf file-node (Predicate "*-read-*")))

; --------------------------------------------------------
; The *-stream-* message will return a handle to a StreamValue
; that will return appended lines, one by one. Referencing the
; stream will dequeue them one at a time, again blocking when
; there's nothing there.

(define txt-stream
	(cog-execute! (ValueOf file-node (Predicate "*-stream-*"))))

; Repeated references to the stream will return single lines from
; the file. Remember to `echo First rule is ... >> /tmp/tail.txt`
txt-stream
txt-stream
txt-stream

; --------------------------------------------------------
; Demo: Perform processing on the stream. For each line of the
; input file, apply a rule to rewrite it into a different form.
;
; To make the stream processor "realistic", the text-dtream source
; needs to placed somewhere, where it can be found.  Notice a trick
; used here: the `DontExec` link shelters the `ValueOf`, and prevents
; it from being executed while the SetValue is running.
(cog-execute!
	(SetValue (Concept "foo") (Predicate "some place")
		(DontExec (ValueOf file-node (Predicate "*-stream-*")))))

; Access to the stream.
(define txt-stream-gen
	(ValueOf (Concept "foo") (Predicate "some place")))

; This rule just makes two copies of each input line, interleaving
; it with other text. The `txt-stream-gen` is the source of input.
(define rule-applier
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
		txt-stream-gen))

; Run the rule, once.
(cog-execute! rule-applier)

; Repeat it again, over and over. This will block, unless the
; file is appended to. This will continue, until the text stream
; is sent the `*-close-*` message. If blocked, then the `*-close-*`
; has to be sent from another thread.
(cog-execute! rule-applier)
(cog-execute! rule-applier)
(cog-execute! rule-applier)
(cog-execute! rule-applier)
(cog-execute! rule-applier)
(cog-execute! rule-applier)

; --------------------------------------------------------
; The End! That's All, Folks!
