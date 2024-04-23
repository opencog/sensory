;
; irc-api.scm -- IRC API demo
;
; Demo of the basic IRC interface. The API offers a simple way to
; connect to IRC and exchange messages.
;
(use-modules (opencog) (opencog exec) (opencog sensory))

; --------------------------------------------------------
; Basic demo: Open a connection to an IRC node. The connection
; is given by a URL of the form
; irc://nick[:pass]@host[:port]
;
(define irc-stream
	(cog-execute!
		(Open
			(Type 'IRChatStream)
			(SensoryNode "irc://botty@irc.libera.chat:6667"))))

; Repeated references to the stream will return single lines from
; the file.
irc-stream

; Place the stream at a "well-known location". This will allow
; pure-Atomese access to the stream, thus allowing it to be
; written to.
(cog-set-value!
	(Anchor "IRC Bot") (Predicate "tester") irc-stream)

; Join an IRC channel
(cog-execute!
	(WriteLink
		(ValueOf (Anchor "IRC Bot") (Predicate "tester"))
		(List (Concept "JOIN") (Concept "#opencog"))))

; Say something on that channel
(cog-execute!
	(WriteLink
		(ValueOf (Anchor "IRC Bot") (Predicate "tester"))
		(List (Concept "PRIVMSG") (Concept "#opencog")
			(Concept "Here's a bunch of words I want to say"))))

; ------------------------------------------------------------
; The above explicitly provides IRC commands and text as Atoms.
; For agents, it is more convenient to flow these as a stream.
; For this purpose, create a WriteLink that, when executed, will
; copy from the input stream to the IRC stream.
(define writer
	(WriteLink
		(ValueOf (Anchor "IRC Bot") (Predicate "tester"))
		(ValueOf (Anchor "Stuff to say") (Predicate "say key"))))

; From this point on, commands are streamed by placing them
; onto the input key, and then executing the writer.
; This time, the stream is a StringValue vector, instead of the
; ConceptNodes above.

; Specify a channel to join
(cog-set-value!
	(Anchor "Stuff to say") (Predicate "say key")
	(StringValue "JOIN" "#opencog"))

; Join that channel
(cog-execute! writer)

; Specify something to say.
(cog-set-value!
	(Anchor "Stuff to say") (Predicate "say key")
	(StringValue "PRIVMSG" "#opencog" "I have something to say"))

; Say something
(cog-execute! writer)

; ------------------------------------------------------------
; Low-level access. Not recommended; provided for debugging.
; Debug log is being written to /tmp/irc-dbg.log

(define bot (ValueOf (Anchor "IRC Bot") (Predicate "tester")))

; low-leve command: join a channel
(cog-execute! (Write bot (List (Concept "JOIN #opencog"))))

; Low-level command: change a nick.
(cog-execute! (Write bot (List (Concept "NICK dorkbot"))))

; Low-level command: set channel topic.
(cog-execute! (Write bot (List (Concept "TOPIC #opencog Who's an operator now"))))

(cog-execute! (Write bot (List (Concept "HELP"))))

; HELP is one of the IRC server commands.

; --------------------------------------------------------
; The End! That's All, Folks!
