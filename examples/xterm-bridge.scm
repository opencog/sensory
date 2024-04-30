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
; Create a pair of xterms; place them where they can be found.

(cog-execute!
	(SetValue
		(Anchor "streams") (Predicate "term A")
		(Open (Type 'TerminalStream))))

(cog-execute!
	(SetValue
		(Anchor "streams") (Predicate "term B")
		(Open (Type 'TerminalStream))))

; Create two copiers. Executing each of these once will result
; in the copy of exactly one line of text.
(define copy-b-to-a
	(WriteLink
		(ValueOf (Anchor "streams") (Predicate "term A"))
		(ValueOf (Anchor "streams") (Predicate "term B"))))

(define copy-a-to-b
	(WriteLink
		(ValueOf (Anchor "streams") (Predicate "term B"))
		(ValueOf (Anchor "streams") (Predicate "term A"))))

; Place the copiers into infinite loops, so that they copy forever.
; Two loops are needed, as each copier will block, waiting for input.

(define do-exit-loop #f)
(define (inf-loop COPIER)
   (cog-execute! COPIER)
   (if (not do-exit-loop) (inf-loop COPIER)))

(define thread-one #f)
(define thread-two #f)
(define (exit-loop)
   (set! do-exit-loop #t)
   (join-thread thread-one)
   (join-thread thread-two)
   (format #t "Exited the loops\n")
   (set! do-exit-loop #f))

; Set the loops running.
(define thread-one (call-with-new-thread (lambda () (inf-loop copy-b-to-a))))
(define thread-two (call-with-new-thread (lambda () (inf-loop copy-a-to-b))))

; Exit, if desired. May have to do some input in each, to unclog things.
; (exit-loop)

; --------------------------------------------------------
; The End! That's All, Folks!
