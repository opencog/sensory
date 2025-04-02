; 
; hamming.scm
; Attempt to compute hamming distance between two sparse vectors.

(use-modules (opencog) (opencog exec) (opencog sensory))

; Low-level tool to declare a relationship R(A,B)
; R, A, B are strings
(define (rela R A B)
	(Edge (Predicate R) (List (Item A) (Item B))))

; Setup data. Some bogus dependency parse.
(rela "word-pair" "the" "dog")
(rela "word-pair" "dog" "chased")
(rela "word-pair" "the" "cat")
(rela "word-pair" "chased" "cat")
(rela "word-pair" "chased" "around")
(rela "word-pair" "the" "house")
(rela "word-pair" "around" "house")
(rela "word-pair" "cat" "around")
(rela "word-pair" "HEAD" "dog")
(rela "word-pair" "HEAD" "chased")

; Create a pattern to for left and right sets.
; The left and right marginal vectors. Unordered sets, actually.
(define lvar (Variable "$left"))
(define rvar (Variable "$right"))
(define marginals
	(Query
		(VariableList
			(TypedVariable lvar (Type 'Item))
			(TypedVariable rvar (Type 'Item)))
		(Present
			(Edge (Predicate "word-pair") (List lvar rvar)))
		lvar rvar))

; Run it
(cog-execute! marginals)
(format #t "The left basis is ~A\n" (cog-value marginals lvar))
(format #t "The right basis is ~A\n" (cog-value marginals rvar))


(define vec-foo (LinkValue
	(Item "a") (Item "c") (Item "e") (Item "f") (Item "g")))

(define vec-bar (LinkValue
	(Item "a") (Item "b") (Item "c") (Item "g") (Item "h")))

; Place data somewhere.
(cog-set-value! (Anchor "foo") (Predicate "foo-key") vec-foo)
(cog-set-value! (Anchor "bar") (Predicate "bar-key") vec-bar)

; We can get sizes of sets.
(cog-execute!
	(SizeOf (ValueOf (Anchor "foo") (Predicate "foo-key"))))

(cog-execute!
	(SizeOf (ValueOf (Anchor "bar") (Predicate "bar-key"))))

; Define an object that can compute Hamming distance.


