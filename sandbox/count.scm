;
; count.scm
; Attempt to count things.
(use-modules (opencog))

(define get-all-types
	(Query 
		(Variable "$atom") ; vardecl
		(Variable "$atom") ; match anything
		(TypeOf  (Variable "$atom")) ; rewrite
	))

(cog-execute! get-all-types)

