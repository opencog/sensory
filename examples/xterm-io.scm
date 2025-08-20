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
(use-modules (opencog) (opencog exec) (opencog sensory))

; --------------------------------------------------------
; Create an xterm for direct I/O. Sending the TerminalNode object the
; *-open-* message will create an xterm window. After this, text can be
; read from the terminal, and sent to the terminal.
;
; Be sure that xterm is installed, else this won't work!
(define xterm (TerminalNode "foo"))
(cog-set-value! xterm (Predicate "*-open-*") (VoidValue))

; Sending the *-read-* message to the object will read a single line of
; text from the xterm window. Each call will block (appear to hang),
; until something is typed into the terminal, followed by a carriage
; return. The interface is line-oriented, you have to hit enter to
; unblock and sent the text.
(define reader (ValueOf (TerminalNode "foo") (Predicate "*-read-*")))
(cog-execute! reader)
(cog-execute! reader)
(cog-execute! reader)
(cog-execute! reader)
(cog-execute! reader)

; Typing a ctrl-D into the terminal will close it, returning an empty
; stream. (aka "end of file")

; The reader above works differently from the TextFileNode. Because
; text files are static, it is safe to have multiple file readers
; running at the same time, and therefore it makes sense to return a
; TextReader stream, and have the stream do the read automatically.
;
; Here, by contrast, each read consumes a line from the xterm, and
; thus, we can't actually have multiple concurrent readers. Thus, the
; API provides a read function that must be called explicitly, to get
; each line.

; Writing proceeds in a manner similar to the TextFileNode.
(cog-set-value! xterm (Predicate "*-write-*")
	(StringValue "Hello there!\n"))

; --------------------------------------------------------
; Demo: Perform indirect streaming. The text to write will be placed as
; a StringValue at some location, and writing will be done from there.

(cog-set-value!
	(Concept "source") (Predicate "key")
	(StringValue
		"Some text\n"
		"Without a newline "
		"after it\n"
		"Goodbye!\n"))

; Define the writer.
(define writer
	(SetValue xterm (Predicate "*-write-*")
		(ValueOf (Concept "source") (Predicate "key"))))

; Write it out.
(cog-execute! writer)
(cog-execute! writer)
(cog-execute! writer)

; --------------------------------------------------------
; The End! That's All, Folks!
