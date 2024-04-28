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

; Open filesystem node, and anchor it.
(cog-execute!
	(SetValue
		(Anchor "xplor") (Predicate "fsys")
		(Open (Type 'FileSysStream) (Sensory "file:///tmp"))))

-----------------------------------------------------------
; Demo the basic commands as a "human" would use them.
; These are, of course, hopelessly verbose for a human, but that
; is beside the point; the goal is to have a self-describing
; interface.
(cog-execute! (Write (ValueOf (Anchor "xplor") (Predicate "fsys"))
	(Item "pwd")))

(cog-execute! (Write (ValueOf (Anchor "xplor") (Predicate "fsys"))
	(Item "ls")))

(cog-execute! (Write (ValueOf (Anchor "xplor") (Predicate "fsys"))
	(List (Item "cd") (Item "file:///home"))))

(cog-execute! (Write (ValueOf (Anchor "xplor") (Predicate "fsys"))
	(Item "ls")))

; Throws because wrong number of args. XXX FIXME
(cog-execute! (Lookup (Type 'TextFileStream)))

; --------------------------------------------------------
; The End! That's All, Folks!
