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

; Do it again and again. Keep doing it untile one of these hangs.
; When it hangs, it will wait until more text is appended to the file.
; Thus: `echo "gaba gaba hey" >> /tmp/tail.txt` will unstick it.
(cog-execute! (ValueOf file-node (Predicate "*-read-*")))
(cog-execute! (ValueOf file-node (Predicate "*-read-*")))
(cog-execute! (ValueOf file-node (Predicate "*-read-*")))
(cog-execute! (ValueOf file-node (Predicate "*-read-*")))

; --------------------------------------------------------
; The *-stream-* message will return a handle to the QueueValue
; that is accumulating the append events. These can be dequeued one
; at a time, again hanging when there's nothing there.

(define txt-stream
	(cog-execute! (ValueOf file-node (Predicate "*-stream-*"))))

; Repeated references to the stream will return single lines from
; the file.
txt-stream
txt-stream
txt-stream

; --------------------------------------------------------
; Demo: Perform indirect streaming. The text-stream will be placed
; as a Value on some Atom, where it can be accessed and processed.
;
; Anchor the text stream at "some place", where it can be found.
(cog-execute!
	(SetValue (Concept "foo") (Predicate "some place")
		(ValueOf file-node (Predicate "*-stream-*"))))

; The stream can be accessed by just fetching it from "some place",
; the location it is anchored at.
(define txt-stream-gen
	(ValueOf (Concept "foo") (Predicate "some place")))

; --------------------------------------------------------
; Demo: Perform processing on the stream. For each line of the
; input file, apply a rule to rewrite it into a different form.

; This rule just makes two copies of each input line, interleaving
; it with other text. It is built with the same ValueOf promise, as
; above.
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

; Repeat it again, over and over, till the end-of-file.
(cog-execute! rule-applier)
(cog-execute! rule-applier)
(cog-execute! rule-applier)
(cog-execute! rule-applier)
(cog-execute! rule-applier)
(cog-execute! rule-applier)

; --------------------------------------------------------
; The End! That's All, Folks!
