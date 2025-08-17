;
; xterm-io.scm -- xterminal I/O demo
;
; Demo showing how to interact directly with the text I/O subsystem,
; via an old-fashioned xterm. (Be sure to install xterm, first!)
;
; The xterm interface is the simplest-possible non-null device. It has
; only four valid operations: Lookat, Open, (read) and Write. There is
; nowhere to go, nothing to do, except to exchange text with an xterm.
;
; It is kind-of like working with a single file, but more fun because
; its interactive. It also eliminates the complexity of walking through
; the file system. It is kind of like chatting with IRC, but without any
; of the complexity of IRC.
;
; This demo is almost identical to the file-read and file-write demos,
; and provides a basic sanity check that everything works.
;
; Review the `xterm-bridge.scm` demo next. It builds a pipeline
; connecting two xterms.
;
(use-modules (opencog) (opencog exec) (opencog sensory-v0))

; --------------------------------------------------------
; Create an xterm for direct I/O. Executing the OpenLink will return
; a stream value that deliver text strings typed into the xterm window,
; and can print text to that window. Be sure that xterm is installed,
; else this won't work!
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

; Typing a ctrl-D into the terminal will close it, returning an empty
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
; Look at available commands. For now, nothing to be done with these,
; that comes in a later demo. Note that there are only two Sections,
; and each Section has a minimal number of Connectors.

(cog-execute! (Lookat (Type 'TerminalStream)))

; --------------------------------------------------------
; The End! That's All, Folks!
