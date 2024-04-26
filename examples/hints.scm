;
; hints.scm -- Coding hints and recipies.
;
; A collection of minor examples showing various maybe-useful recipies.
;
(use-modules (srfi srfi-1))
(use-modules (opencog) (opencog exec) (opencog sensory))

; -------------------------------------------------------
; A tutorial on Atomese pipelines.
; So here's a sequence of tricks.
; -----------------
; Trick: calling scheme code (this works for python, too)

(define exo
	(ExecutionOutput
		(GroundedSchema "scm: foo")                ; the function
		(List (Concept "bar") (Concept "baz"))))   ; the arguments

(define (foo x y)
	(format #t "I got ~A and ~A\n" x y)
	(Concept "foo reply"))

; Run it and see.
(cog-execute! exo)

; -----------------
; Just like above, but use the function to read the stream

(define (process-stream stm)
	(format #t "Stream is ~A\n" stm)
	(define retv (cog-execute! stm))
	(format #t "Stream read gave ~A\n" retv)
	retv)

(define read-stream
	(ExecutionOutput
		(GroundedSchema "scm: process-stream")
		bot-read))

(cog-execute! read-stream)

; -----------------
; Same as above, but this time in pure Atomese

(define exocet
	(ExecutionOutput
		(Lambda (VariableList
			; Expect three arguments
			(Variable "$from") (Variable "$to") (Variable "$msg"))
			; Body that arguments will be beta-reduced into.
			(List (Item "foobar") (Variable "$to")))

		; Arguments that the lambda will be applied to.
		(List
			(Concept "first") (Concept "second") (Concept "third"))))

(cog-execute! exocet)

; -----------------
; As above

; This doen't work because the
(define exorcist
	(ExecutionOutput
		(Lambda (Variable "$foo")
			; Body that arguments will be beta-reduced into.
			(List (Item "foobar") (Variable "$foo")))
		 (LinkSignature (Type 'LinkValue) bot-raw)))

(cog-execute! exorcist)

; -----------------

; The End. That's all, folks!
; -------------------------------------------------------
