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
(use-modules (srfi srfi-1))

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

; Get the description of what this sensory device provides.
(define fsys-descr
	(cog-execute! (Lookup (Type 'FileSysStream))))

; Print it. It should be a ChoiceLink of a bucn of sections
fsys-descr

; Brute-force just get the first choice. It's a Section with
; a ConnectorSeq on it.
(define some-cmd-descr (cog-value-ref fsys-descr 0))

; Take a look
some-cmd-descr

; Get the connector sequence
(define conn-seq (cog-value-ref some-cmd-descr 1))

; Convert it to a scheme list
(define conn-list (cog-value->list conn-seq))

; Brute-force walk over the connectors, and extract the command
; components.
(define cmd-seq
	(filter-map
		(lambda (cnctr)
			(if (equal? (gar cnctr) (Sex "command"))
				(gdr cnctr)
				#f))
		conn-list))

; Assemble the command
(define some-cmd
	(cog-execute!
		(LinkSignature
			(car cmd-seq)
			(DontExec (ValueOf (Anchor "xplor") (Predicate "fsys")))
			(cdr cmd-seq))))

; Take a look at what we got
some-cmd

; Run it. This assume the Open was done earlier.
(cog-execute! some-cmd)

; --------------------------------------------------------

(cog-execute!
	(Filter
		(Rule
			(Choice (Variable "$x"))
			(List (Variable "$x")))
		fsys-descr))

(cog-execute!
	(Filter
		(Section
			(Type 'Item)
			(Type 'ConnectorSeq))
		fsys-descr))

; --------------------------------------------------------

; Throws because wrong number of args. XXX FIXME
(cog-execute! (Lookup (Type 'TextFileStream)))

; --------------------------------------------------------
; The End! That's All, Folks!
