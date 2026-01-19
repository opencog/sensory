
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

(cog-execute!
	(SetValue
      (RocksStorageNode "rocks:///tmp/foo")
      (Predicate "*-store-atom-*")
		(Concept "I'm from storage")))

