;
; xterm-io.scm -- xterminal I/O demo
;
; Demo showing how to interact directly with the text I/O subsystem,
; via an old-fashioned xterm.
;
; This is the simplest interface in the system: its sort of like IRC,
; but without any of the complexity. It's sort of like the file-I/O
; demos, but more fun than working with a single static file. You can
; directly interact with an agent, using this device.
;
; The first part of the demo is almost identical to the file-read and
; file-write demos, and provides a basic sanity check that everything
; works.
;
; The second part of the demo demonstrates auto-pipelining.
; Under construction.
;
(use-modules (opencog) (opencog exec) (opencog sensory))

; --------------------------------------------------------
; Create an xterm for direct I/O. Executing the OpenLink will return
; a stream value that deliver text strings typed into the xterm window,
; and can print text to that window.
(define term-stream
	(cog-execute! (Open (Type 'TerminalStream))))

; Repeated references to the stream will return single lines from
; the xterm window. These will hang, unless/until something is typed
; into the terminal. The interface is line-oriented, you have to hit
; enter at the end-of line.
term-stream
term-stream
term-stream
term-stream

; Typing a ctrl-D into ther terminal will close it, returning an empty
; stream. (aka "end of file")

; A WriteLink consists of two parts: where to write, and what
; to write. Since the write cursor is a Value, not an Atom, we
; cannot specify it directly. Thus, we place it at an anchor
; point. (Neither the name "xterm anchor", nor the key "output place"
; matter. They can be anything, and any atom can be used in their
; place, including Links.)
(cog-set-value!
	(Concept "xterm anchor") (Predicate "output place") term-stream)

; Create a WriteLink
(define writer
	(WriteLink
		(ValueOf (Concept "xterm anchor") (Predicate "output place"))
		(Concept "stuff to write to the terminal\n")))

; Write stuff to the terminal.
(cog-execute! writer)

; Do it a few more times.
(cog-execute! writer)
(cog-execute! writer)
(cog-execute! writer)

; --------------------------------------------------------
; Demo: Perform indirect streaming. The text to write will be placed as
; a StringValue at some location, and writing will be done from there.

(cog-set-value!
	(Concept "source") (Predicate "key")
	(StringValue
		"some text\n"
		"without a newline"
		"after it\n"
		"Goodbye!\n"))

; Redefine the writer.
(define writer
	(WriteLink
		(ValueOf (Concept "xterm anchor") (Predicate "output place"))
		(ValueOf (Concept "source") (Predicate "key"))))

; Write it out.
(cog-execute! writer)
(cog-execute! writer)
(cog-execute! writer)

; --------------------------------------------------------
; Look at available commands.

(cog-execute! (Lookup (Type 'TerminalStream)))

; --------------------------------------------------------
; The End! That's All, Folks!
