;
; filesys.scm -- filesystem demo
;
; Demo the introspection (reflection) API for file system navigation.
; The goal is to demonstrate how an agent can find out what an API
; looks like, for some sensory environment, and what the motor commands
; are, to move around in that environment. The "environment" here
; happens to be the file system. The motor commands correspond to
; a small subset of the traditional unix filesystem commands. An agent
; can discover what these are, by puttering around in the API. This demo
; shows how.
;
; Under construction. Uses ideas sketched in the README in
; opencog/atoms/irc/README.md
;
(use-modules (opencog) (opencog exec) (opencog sensory))

; -----------------------------------------------------------
; Preliminary setup.

; Open filesystem node, and anchor it.
(cog-execute!
	(SetValue
		(Anchor "xplor") (Predicate "fsys")
		(Open (Type 'FileSysStream) (Sensory "file:///tmp"))))

; In principle, the agent should do the above after discovery.
; For now, for this demo, just hard-code it, as above.

; -----------------------------------------------------------
; Demo the basic commands as a "human" would use them.
; These are, of course, hopelessly verbose for a human, but that
; is beside the point; the goal is to have a self-describing
; interface. In some programming languages, this is called
; "introspection" or "reflection".
(cog-execute! (Write (ValueOf (Anchor "xplor") (Predicate "fsys"))
	(Item "pwd")))

(cog-execute! (Write (ValueOf (Anchor "xplor") (Predicate "fsys"))
	(Item "ls")))

(cog-execute! (Write (ValueOf (Anchor "xplor") (Predicate "fsys"))
	(List (Item "cd") (Item "file:///home"))))

(cog-execute! (Write (ValueOf (Anchor "xplor") (Predicate "fsys"))
	(Item "ls")))

; --------------------------------------------------------
; Now lets look at how the API for the above is represented.

; Get the stream description
(define fsys-descr
	(cog-execute! (Lookup (Type 'FileSysStream))))

fsys-descr

; --------------------------------------------------------

; Throws because wrong number of args. XXX FIXME
(cog-execute! (Lookup (Type 'TextFileStream)))

; --------------------------------------------------------
; The End! That's All, Folks!
