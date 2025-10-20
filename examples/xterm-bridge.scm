;
; xterm-bridge.scm -- copying between a pair of xterms.
;
; Demo showing how to set up a pair of pipelines between two xterms.
; Text typed into one appears in the other, and vice-versa.
;
; The bridge is constructed manually, without any automation. It is a
; two-lane bridge, as each xterm can be both a source and also a sink
; for text, and so there will be two connections, input to output.
;
; The eventual goal, in a later demo, is to automatically construct
; this bridge (these kinds of bridges), by linking together all open
; connectors on all device descriptions. This demo shows one of the
; simplest examples of what such running links could look like.
;
; For a more basic review of how to read and write from inputs and
; outputs, review the `xterm-io.scm` demo first.
;
; Minor warning: something with xterm might be slightly flakey, in
; that sometimes, text is copied twice. This is not a bug in the
; Atomese code, but some weirdness with xterm and pseudo-terminal
; /dev/pts pipes. Not worth debugging, since this is not a critical
; component. So, just FYI. See `fgets_unlocked` in the source for more.

(use-modules (opencog) (opencog exec) (opencog sensory))

; --------------------------------------------------------
; Create a pair of xterms; open them for reading and writing.

(define axterm (TerminalNode "term A"))
(define bxterm (TerminalNode "term B"))

(cog-execute!
	(SetValue axterm (Predicate "*-open-*") (Type 'StringValue)))
(cog-execute!
	(SetValue bxterm (Predicate "*-open-*") (Type 'StringValue)))

; Create two copiers. Executing each of these once will copy exactly
; one message from one terminal to the other.

(define copy-one-b-to-a
	(SetValue axterm (Predicate "*-write-*")
		(ValueOf bxterm (Predicate "*-read-*"))))

(define copy-one-a-to-b
	(SetValue bxterm (Predicate "*-write-*")
		(ValueOf axterm (Predicate "*-read-*"))))

; --------------------------------------------------------
; The current xterm reader API is NOT streaming, it is line-at-a-time.
; Thus, to get an endlessly running system, a pair of infinite loops
; need to be constructed, with each loop copying on one direction.
; There are three ways to build such loops:
; 1) Write them in scheme
; 2) Write them in Atomese
; 3) Use an Atomese shim that converts line-at-a-time interfaces to
;    streams.
; All three approaches will be illustrated.
;
; --------------------------------------------------------
; Option 1) scheme loops: this works, but violates the principle that
; everything should be done in Atomese. The goal, as always, is to write
; all dataflows in Atomese, as graphs, and not in scheme/python/etc.

(define (b-to-a-loop) (cog-execute! copy-one-b-to-a) (b-to-a-loop))
(define (a-to-b-loop) (cog-execute! copy-one-a-to-b) (a-to-b-loop))

; Because we want to run both loops at the same time, they each need to
; go into their own threads.
(call-with-new-thread b-to-a-loop)
(call-with-new-thread a-to-b-loop)

; That's it! Try it! Anything typee in one terminal will now be echoed
; in the other. The threads will run forever. To stop things, you have
; to close the terminals:
(cog-set-value! axterm (Predicate "*-close-*") (VoidValue))
(cog-set-value! bxterm (Predicate "*-close-*") (VoidValue))

; Sometimes, when switching from one terminal to the other, you
; might see the echoed text duplicated. This is a bug. It's due to
; some weird fgets-threading-locking bug, see TerminalStream.cc
; for details. Low priority, so not fixed.
; --------------------------------------------------------
; Option 2) Same as Option 1) but in pure Atomese.

; Define two infinite loops, via tail recursion. The PureExecLink
; executes everything that it wraps in the given AtomSpace. We do
; all the work in *this* Atomspace; this must be given explicitly,
; as otherwise the execution is done "purely", in a temporary
; scratch AtomSpace.
(Define
	(DefinedProcedure "b-to-a-tail")
	(PureExec (cog-atomspace) copy-one-b-to-a (DefinedProcedure "b-to-a-tail")))

(Define
	(DefinedProcedure "a-to-b-tail")
	(PureExec (cog-atomspace) copy-one-a-to-b (DefinedProcedure "a-to-b-tail")))

; If you mess with the above, you will discover that redefines are not
; allowed. To work around that, extract the definition, like so:
;    (cog-extract-recursive! (DefinedProcedure "b-to-a-tail"))

; One infinite loop at a time can be run like so:
;    (cog-execute! (DefinedProcedure "b-to-a-tail"))
; but we want to run both at once. Do this by running them in two
; threads
(cog-execute! (ExecuteThreaded
	(DefinedProcedure "b-to-a-tail")
	(DefinedProcedure "a-to-b-tail")))

; That's it! Try it! Anything typee in one terminal will now be echoed
; in the other, just as before. Closing the terminals will also end
; the threads they are running in.
(cog-set-value! axterm (Predicate "*-close-*") (VoidValue))
(cog-set-value! bxterm (Predicate "*-close-*") (VoidValue))

; --------------------------------------------------------
; Option 3) Convert the line-by-line reader to a stream, and then
; let the implicit infinite loops in the stream do all the work.
; This is not hard, (in fact, it's done automatically) but we'll
; take the long road and explain this in detail.

; Wrap the terminal with a reader stream.  Each read from the stream
; will cause a *-read-* message to the sent to the terminal object.
; The ReadStream value is generic: it will work with *any* Sensory
; object (since all SensoryNode objects support *-read-* messages).
(define areader (ReadStream axterm))

; Try it. Each reference returns one line from the xterm. If there's
; nothing to return, the reference will hang, until you type something
; at the xterm, and hit enter.
areader
areader
areader
areader
areader

; In analogoy to copy-a-to-b, we would like to be able to declare
;
;    (SetValue bxterm (Predicate "*-write-*")
;        (ReadStream axterm))
;
; so that everything streamed from A goes to B. But the above expression
; is not valid: ReadStream is a Value, not an Atom. What's needed is an
; Atom that constructs ReadStreams. Well, there is one: the StreamNode.
; Sending it the *-stream-* message will return a ReadStream that wraps
; the *-read-* message on the same object. Anything inheriting from
; StreamNode gets this wrapper "for free". This includes the
; TextStreamNode, from which the TerminalNode is derived. In short, the
; TerminalNode has this built in.
;
; It can be used in the same way as in the file reader stream demo:
(define txt-stream-gen (ValueOf axterm (Predicate "*-stream-*")))

; Note that each call blocks, until something is typed into terminal A.
(cog-execute! txt-stream-gen)
(cog-execute! txt-stream-gen)
(cog-execute! txt-stream-gen)
(cog-execute! txt-stream-gen)

; The bridge is now straight-forward: Take the copy-one pattern,
; and replace *-read-* by *-stream-*
(define stream-b-to-a
	(SetValue axterm (Predicate "*-write-*")
		(ValueOf bxterm (Predicate "*-stream-*"))))

(define stream-a-to-b
	(SetValue bxterm (Predicate "*-write-*")
		(ValueOf axterm (Predicate "*-stream-*"))))

; Either of the above, set in motion, will run forever, copying from
; source to destination. This is done with an infinite loop in the
; StreamNode::write() method.
;
: As these are both inf loops, they will not return to the caller
; until the corresponding input stream terminates. To run both loops
; at the same time, put them in different threads:

(cog-execute! (ExecuteThreaded stream-b-to-a stream-a-to-b))

; That's it. Text typed into either terminal is sent to the other.

; --------------------------------------------------------
; The End! That's All, Folks!
