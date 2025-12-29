;
; count.scm
; Attempt to count things.
(use-modules (opencog))

(define get-all-types
	(Query
		(Variable "$atom") ; vardecl
		(Variable "$atom") ; match anything
		(TypeOf (DontExec (Variable "$atom"))) ; rewrite
	))

(cog-execute! get-all-types)

(define get-all-atoms
	(Meet
		(Variable "$atom") ; vardecl
		(Variable "$atom") ; match anything
	))

(cog-execute! get-all-atoms)

(define get-all-queries
	(Query
		(TypedVariable (Variable "$atom") (Type 'Query))
		(Variable "$atom") ; match anything
		(DontExec (Variable "$atom")) ; rewrite
	))

(cog-execute! get-all-queries)


