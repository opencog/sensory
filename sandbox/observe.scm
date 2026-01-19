
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

