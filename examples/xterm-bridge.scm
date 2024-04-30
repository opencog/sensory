;
; xterm-bridge.scm -- copying between a pair of xterms.
;
; Demo showing how to set up a pair of pipelines between two xterms.
; Text typed into one appears in the other, and vice-versa.
;
; This manually constructs a bridge between the two devices. The
; eventual goal, of a later demo, is to automatically construct this
; bridge, be linking together all open connectors on all device
; descriptions. This demo shows what that bridge would look like.
;
; It might be helpful to review the `xterm-io.scm` demo first.
;
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
(define thread-one (call-with-new-thread (lambda () (inf-loop copy-a-to-b))))

; Exit, if desired. May have to do some input in each, to unclog things.
; (exit-loop)

; --------------------------------------------------------
; The End! That's All, Folks!
