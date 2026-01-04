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

; -------------------------------------------
(use-modules (opencog))

(PipeLink
	(Name "get-all-atoms")
	(Meet
		(Variable "$atom") ; vardecl
		(Variable "$atom") ; match anything
	))

; (cog-execute! (Name "get-all-atoms"))

(PipeLink
	(Name "get-types")
	(Filter
		(Rule
			(TypedVariable (Variable "$atom") (Type 'Atom)) ; vardecl
			(Variable "$atom") ; body - accept everything
			(TypeOf (DontExec (Variable "$atom"))))
		(Name "get-all-atoms")))

; (cog-execute! (Name "get-types"))

(Pipe
	(Name "count-types")
	(Filter
		(Rule
			(TypedVariable (Variable "$typ") (Type 'Type)) ; vardecl
			(Variable "$typ") ; body - accept everything
			(IncrementValue (Variable "$typ") (Predicate "cnt") (Number 0 0 1)))
		(Name "get-types")))

; Must actually run this one...
; Can we have an Atom that just runs where created?
(cog-execute! (Name "count-types"))

(Pipe
	(Name "unique-types")
	(CollectionOf (TypeNode 'UnisetValue)
		(Name "get-types")))

; (cog-execute!  (Name "unique-types"))

(DefineLink
	(DefinedPredicate "count-order")
	(Lambda
		(VariableList (Variable "left") (Variable "right"))
		(Not
			(GreaterThan
				(ElementOf (Number 2)
					(ValueOf (Variable "left") (Predicate "cnt")))
				(ElementOf (Number 2)
					(ValueOf (Variable "right") (Predicate "cnt")))))))

(define x
	(SortedValue
		(DefinedPredicate "count-order")
		(Name "unique-types")))


; Debug print
(cog-execute!
	(Filter
		(Rule
			(TypedVariable (Variable "$typ") (Type 'Type)) ; vardecl
			(Variable "$typ") ; body - accept everything
			(LinkSignature (Type 'LinkValue)
				(Variable "$typ")
				(ValueOf (Variable "$typ") (Predicate "cnt"))))
		(Name "unique-types")))


(cog-execute!
	(CollectionOf 
(TypeNode 'SortedValue)
		(Name "unique-types")))

