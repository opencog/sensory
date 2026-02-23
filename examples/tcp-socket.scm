;
; tcp-socket.scm -- TCP socket I/O demo
;
; Demo showing how to interact with a TCP socket. This is similar to
; the unix-socket demo, but uses TCP/IP instead of a Unix domain socket.
; An external user can connect to the socket over the network (or
; localhost) using `socat` or a similar tool.
;
; The socket interface implements the ObjectNode messages:
; *-open-*, *-close-*, *-read-*, *-write-*, and the rest. Text is
; exchanged line-by-line.
;
; To use this demo:
; 1. Start a guile shell, and cut-n-paste from this demo to the shell.
; 2. The open command creates the socket and starts listening.
; 3. In another terminal, run:  socat - TCP:localhost:5000
; 4. The first read will block until the socat client connects and
;    sends a line. After that, you can type in the socat terminal
;    and read here, or write from here and see it in the socat terminal.
;
; This demo is almost identical to the unix-socket-io demo, and provides
; a basic sanity check that everything works.
;
(use-modules (opencog) (opencog sensory))

; --------------------------------------------------------
; Create a TCP socket. Sending the TcpSocketNode object the
; *-open-* message will create the socket, bind it, and start listening.
; The open returns immediately; the accept happens on the first read.
;
; After opening, run in another terminal: socat - TCP:localhost:5000
(PipeLink (NameNode "tsock") (TcpSocketNode "tcp://0.0.0.0:5000"))
(Trigger
	(SetValue (NameNode "tsock") (Predicate "*-open-*") (Type 'StringValue)))

; --------------------------------------------------------
; Sending the *-read-* message to the object will read a single line of
; text from the socket. Each call will block (appear to hang), until
; something is typed into the socat session, followed by a carriage
; return. The interface is line-oriented: you have to hit enter to
; unblock and send the text.
(Pipe (Name "reader") (ValueOf (NameNode "tsock") (Predicate "*-read-*")))
(Trigger (Name "reader"))
(Trigger (Name "reader"))
(Trigger (Name "reader"))

; If the socat client disconnects (ctrl-C), the read returns an empty
; string (EOF).

; --------------------------------------------------------
; Writing proceeds in a manner similar to the xterm and TextFileNode.
(Trigger (SetValue (NameNode "tsock") (Predicate "*-write-*")
	(Item "Hello to everyone!\n")))

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
(Define
	(DefinedSchema "writer")
	(SetValue (NameNode "tsock") (Predicate "*-write-*")
		(ValueOf (Concept "source") (Predicate "key"))))

; Write it out.
(Trigger (DefinedSchema "writer"))

; --------------------------------------------------------
; Close the socket when done. This cleans up all resources.
(Trigger
	(SetValue (NameNode "tsock") (Predicate "*-close-*") (Item "bye")))

; --------------------------------------------------------
; The End! That's All, Folks!
