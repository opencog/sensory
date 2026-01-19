
(use-modules (opencog))
(AtomSpace (cog-atomspace)) ;; XXX not pure Atomese
(AtomSpace (AtomSpaceOf (Concept "x"))) ;; Yes, creates chold, OK.

; Create just one child atomspace, stash it where we can find it.
(cog-execute! (SetValue
   (AnchorNode "observer")
   (Predicate "*-space-*")
   (AtomSpace (AtomSpaceOf (AnchorNode "observer")))))

(cog-execute! (ValueOf (AnchorNode "observer") (Predicate "*-space-*")))
(cog-new-atom (Concept "x") (... some atomspace))

; ---------------------------

; Create a handle we can hold onto.
; Compact but too circular.

(use-modules (opencog))
(Pipe
	(Name "make-as")
	(AtomSpace (AtomSpaceOf (Name "make-as"))))

; Create a handle we can hold onto.
(Pipe
	(Name "make-as")
	(AtomSpace "boostrap" (AtomSpaceOf (Link))))

; Verify that the handle works as expected
(cog-execute! (Name "make-as"))
(cog-execute! (AtomSpaceOf (Name "make-as")))

(cog-execute! (SetValue
	(RocksStorageNode "rocks:///tmp/foo")
	(Predicate "*-open-*")
	(Name "make-as"))

(cog-prt-atomspace)
(cog-set-atomspace! (cog-execute! (Name "make-as")))


;; The following has unexpected results:
(use-modules (opencog))
(define base-as (cog-atomspace))
(define named-as (AtomSpace (cog-name (cog-atomspace))))
(cog-equal? base-as named-as) ; currently prints #t
(Concept "foo")  ;; add contents to base-as
(cog-prt-atomspace)  ;; view contents of base-as
(cog-set-atomspace! named-as)  ;; change to the other atomspace
(cog-prt-atomspace)    ;; view contents of named-as -- its empty
(Concept "I'm in named")
(cog-prt-atomspace)    ;; view contents of named-as -- its empty
(cog-set-atomspace! base-as)  ;; return to the original as
(cog-prt-atomspace)  ;; view contents -- we can see the "foo" here
(define nuther-as (AtomSpace (cog-name (cog-atomspace))))
(cog-set-atomspace! nuther-as)  ;; return to the original as
(cog-prt-atomspace)  ;; view contents --
;; So clearly these two AtomSpaces are NOT the same; one has
;; (Concept "foo") in it and the other does not. The problem is
;; that they *should have been* the same AtomSpace -- the name
;; should have been enough to guarentee that broth reference the
;; same space. But somehow something is broken... what is broken?


(AtomSpace "bootstrap" (AtomSpaceOf (Link)))

(cog-execute!
	; Execute a sequence of steps to load the boostrap space,
	; and get things started in there.
	(PureExec
		; Step one: Open the StorageNode
		(SetValue
			(RocksStorageNode "rocks:///tmp/foo")
			(Predicate "*-open-*")
			(AtomSpace "bootstrap"))

		; Step two: Load up the child space with data:
		(SetValue
			(RocksStorageNode "rocks:///tmp/foo")
			(Predicate "*-load-atomspace-*")
			(Link))

		; Step three: run something...
		; Where? In the child AtomSpace
		(AtomSpace "bootstrap")))

(define bas (cog-atomspace))
(cog-set-atomspace! (AtomSpace "bootstrap"))

(cog-execute!
	(SetValue
      (RocksStorageNode "rocks:///tmp/foo")
      (Predicate "*-store-atom-*")
		(Concept "I'm from storage")))

(cog-set-atomspace! (AtomSpace "bootstrap"))
(define (ola) (format #t "hello baby!\n"))
(define pip
(Pipe
	(Name "bootloader")
	(ExecutionOutput
		(GroundedSchema "scm:ola")
		(List)))
)
(cog-execute!
	(SetValue
      (RocksStorageNode "rocks:///tmp/foo")
      (Predicate "*-store-atom-*")
		pip))

(cog-execute! (Name "bootloader"))


