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

(define get-types
	(Filter
		(Rule
			(TypedVariable (Variable "$atom") (Type 'Atom)) ; vardecl
			(Variable "$atom") ; body - accept everything
			(TypeOf (DontExec (Variable "$atom"))))
		get-all-atoms))

; Should work just like `get-all-types` Query above.
(cog-execute! get-types)

(define count-types
	(Filter
		(Rule
			(TypedVariable (Variable "$typ") (Type 'Type)) ; vardecl
			(Variable "$typ") ; body - accept everything
			(IncrementValue (Variable "$typ") (Predicate "cnt") (Number 0 0 1)))
		get-types))

(cog-execute! count-types)
