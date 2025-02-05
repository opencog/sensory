; 
; hamming.scm
; Attempt to compute hamming distance between two sparse vectors.

(use-modules (opencog) (opencog exec))

; Setup data.
(define vec-foo (LinkValue
	(Item "a") (Item "c") (Item "e") (Item "f") (Item "g")))

(define vec-bar (LinkValue
	(Item "a") (Item "b") (Item "c") (Item "g") (Item "h")))

; Place data somewhere.
(cog-set-value! (Anchor "foo") (Predicate "foo-key") vec-foo)
(cog-set-value! (Anchor "bar") (Predicate "bar-key") vec-bar)

