;
; filesys.scm -- filesystem demo
;
; Demo agent commands for navigating a filesystem.
;
; Under construction. Uses ideas sketched in the README in
; opencog/atoms/irc/README.md
;
(use-modules (opencog) (opencog exec) (opencog sensory))

; Get the stream description
(define fsys-descr
	(cog-execute! (Lookup (Type 'FileSysStream))))

fsys-descr

; Open
(define fsys-stream
	(cog-execute! (Open (Type 'FileSysStream) (Sensory "file:///tmp"))))

; Anchor
(cog-set-value! (Anchor "xplor") (Predicate "fsys") fsys-stream)

; Write
(cog-execute! (Write (ValueOf (Anchor "xplor") (Predicate "fsys"))
	(Item "pwd")))

(cog-execute! (Write (ValueOf (Anchor "xplor") (Predicate "fsys"))
	(Item "ls")))

; Throws because wrong number of args. XXX FIXME
(cog-execute! (Lookup (Type 'TextFileStream)))

; --------------------------------------------------------
; The End! That's All, Folks!
