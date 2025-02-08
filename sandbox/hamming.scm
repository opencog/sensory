; 
; hamming.scm
; Attempt to compute hamming distance between two sparse vectors.

(use-modules (opencog) (opencog exec))

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

(define vec-foo (LinkValue
	(Item "a") (Item "c") (Item "e") (Item "f") (Item "g")))

(define vec-bar (LinkValue
	(Item "a") (Item "b") (Item "c") (Item "g") (Item "h")))

; Place data somewhere.
(cog-set-value! (Anchor "foo") (Predicate "foo-key") vec-foo)
(cog-set-value! (Anchor "bar") (Predicate "bar-key") vec-bar)

