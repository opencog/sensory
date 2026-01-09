;
; count.scm
; Attempt to count things. Scratchpad.
; Final clean version in atomspace/examples/flow/count-pipeline.scm
;
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
			(LessThan
				(ElementOf (Number 2)
					(ValueOf (Variable "left") (Predicate "cnt")))
				(ElementOf (Number 2)
					(ValueOf (Variable "right") (Predicate "cnt")))))))

(Pipe
	(Name "sorted-types")
	(LinkSignature
		(TypeNode 'SortedValue)
		(DefinedPredicate "count-order")
		(Name "unique-types")))

(cog-execute! (Name "sorted-types"))

; Debug print
(cog-execute!
	(Filter
		(Rule
			(TypedVariable (Variable "$typ") (Type 'Type)) ; vardecl
			(Variable "$typ") ; body - accept everything
			(LinkSignature (Type 'LinkValue)
				(Variable "$typ")
				(ValueOf (Variable "$typ") (Predicate "cnt"))))
		(Name "sorted-types")))

; --------------------
(define (data-printer NAME COUNT)
	(StringValue
		(format #f "Usage count of type: ~A ~A"
			(cog-value-ref COUNT 2)
			(cog-name NAME))))

; --------------------
(cog-execute!
	(Filter
		(Rule
			(TypedVariable (Variable "$typ") (Type 'Type)) ; vardecl
			(Variable "$typ") ; body - accept everything
			(ExecutionOutput
				(GroundedSchema "scm:data-printer")
				(LinkSignature (Type 'LinkValue)
					(Variable "$typ")
					(ValueOf (Variable "$typ") (Predicate "cnt")))))
		(Name "sorted-types")))

(cog-execute!
	(Filter
		(Rule
			(TypedVariable (Variable "$typ") (Type 'Type)) ; vardecl
			; Specify a body that rejects types without counts.
			(And
				(Present (Variable "$typ"))
				(Equal
					(Type 'FloatValue)
					(TypeOf (ValueOf (Variable "$typ") (Predicate "cnt")))))
			(ExecutionOutput
				(GroundedSchema "scm:data-printer")
				(LinkSignature (Type 'LinkValue)
					(Variable "$typ")
					(ValueOf (Variable "$typ") (Predicate "cnt")))))
		(Name "sorted-types")))

(cog-execute!
	(Filter
		(Rule
			(TypedVariable (Variable "$typ") (Type 'Type)) ; vardecl
			; Specify a body that rejects types without counts.
			(And
				(Present (Variable "$typ"))
				(Equal
					(Type 'FloatValue)
					(TypeOf (ValueOf (Variable "$typ") (Predicate "cnt")))))
			(LinkSignature (Type 'LinkValue)
				(Node "Usage count of type: ")
				(Variable "$typ")
				(Node " is equal to ")
				(ElementOf (Number 2)
					(ValueOf (Variable "$typ") (Predicate "cnt")))))
		(Name "sorted-types")))

(cog-execute!
	(Filter
		(Rule
			(TypedVariable (Variable "$typ") (Type 'Type)) ; vardecl
			; Specify a body that rejects types without counts.
			(And
				(Present (Variable "$typ"))
				(Equal
					(Type 'FloatValue)
					(TypeOf (ValueOf (Variable "$typ") (Predicate "cnt")))))
			(LinkSignature (Type 'LinkValue)
				(Node "Usage count of type: ")
				(Variable "$typ")
				(Node " is equal to ")
				(ElementOf (Number 2)
					(ValueOf (Variable "$typ") (Predicate "cnt")))))
		(Name "sorted-types")))


(cog-execute!
	(CollectionOf (Type 'StringValue) (Name "list of structures")))

(cog-execute!
	(Concatenate (Type 'StringValue) (Name "list of structures")))

(cog-execute!
	(LinkSignature
		(Signature
			(Type 'StringValue)
			(Type 'StringValue)
			(Type 'StringValue)
			(Type 'StringValue))
		(Name "list of structures")))


(cog-execute!
	(Filter
		(Rule
			(VariableList
				(Variable "$a")
				(Variable "$b")
				(Variable "$c")
				(Variable "$d"))
			(LinkSignature (Type 'LinkValue)
				(Variable "$a")
				(Variable "$b")
				(Variable "$c")
				(Variable "$d"))
			(LinkSignature (Type 'LinkValue)
				(LinkSignature (Type 'StringValue) (Variable "$a"))
				(LinkSignature (Type 'StringValue) (Variable "$b"))
				(LinkSignature (Type 'StringValue) (Variable "$c"))
				(LinkSignature (Type 'StringValue) (Variable "$d"))))
		(Name "list of structures")))


(cog-execute!
	(LinkSignature (Type 'StringValue) (Number 3)))
;; ---------
		(Rule
			(VariableList
				(TypedVariable (Variable "$typ") (Type 'Type))
				(TypedVariable (Variable "$cnt") (Type 'FloatValue)) ; vardecl
			; Specify a body that rejects types without counts.
			(And
				(Present (Variable "$typ"))
				(Equal
					(Variable "$cnt")
					(ValueOf (Variable "$typ") (Predicate "cnt"))))

GuardLink

(cog-execute!
	(Filter
		(Equal
			(Type 'FloatValue)
			(TypeOf (ValueOf (Variable "$typ") (Predicate "cnt"))))
		(Name "sorted-types")))

; --------------------------
