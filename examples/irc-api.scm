;
; irc-api.scm -- IRC API demo
;
; Demo of the basic IRC interface. The API offers a simple way to
; connect to IRC and exchange messages.
;
; See `irc-echo-bot.scm` for a more complex demo that builds on
; these basic concepts.
;
; HINT: The demo below makes use of NameNodes and DefinedSchemas.
; These can only be defined once, ever. Attempting a redefinition will
; throw an exception. So if you want to experiment, you will need to
; erase these. You can do this with (DeleteRecursive (Name "whatever"))
; or (DeleteRecursive (DefinedSchemaNode "stuffs")).
;
(use-modules (opencog) (opencog sensory))

; --------------------------------------------------------
; Basic demo: Open a connection to an IRC node. The connection
; is given by a URL of the form
; irc://nick[:pass]@host[:port]
;
; The NameNode is just a handy handle that we can use in the
; Atomese code, so that the chatbot name is not hard-coded
; further on in this demo.
(PipeLink
	(NameNode "IRC chat object")
	(IRChatNode "blondie"))

; Open the connection.
(Trigger
	(SetValue (NameNode "IRC chat object") (Predicate "*-open-*")
	(Concept "irc://blondie@irc.libera.chat:6667")))

; Repeated references to the read predicate will return single lines
; from the server.
(Trigger (ValueOf (NameNode "IRC chat object") (Predicate "*-read-*")))

; Join an IRC channel
(Trigger (SetValue (NameNode "IRC chat object") (Predicate "*-write-*")
	(List (Concept "JOIN") (Concept "#opencog"))))

; Say something on that channel
(Trigger (SetValue (NameNode "IRC chat object") (Predicate "*-write-*")
	(List (Concept "PRIVMSG") (Concept "#opencog")
		(Concept "Here's a bunch of words I want to say"))))

; ------------------------------------------------------------
; The above explicitly provides IRC commands and text as Atoms.
; For agents, it is more convenient to flow these as a stream.
; For this purpose, create a SetValue that, when executed, will
; copy from the input key to the IRC stream.
(Define
	(DefinedSchema "writer")
	(SetValue
		(NameNode "IRC chat object") (Predicate "*-write-*")
		(ValueOf (Anchor "Stuff to say") (Predicate "say key"))))

; From this point on, commands are streamed by placing them
; onto the input key, and then executing the writer.

; Specify a channel to join
(Trigger (SetValue
	(Anchor "Stuff to say") (Predicate "say key")
	(List (Concept "JOIN") (Concept "#opencog"))))

; Join that channel
(Trigger (DefinedSchema "writer"))

; Specify something to say.
(Trigger (SetValue
	(Anchor "Stuff to say") (Predicate "say key")
	(List (Concept "PRIVMSG") (Concept "#opencog")
		(Concept "I have something to say"))))

; Say something
(Trigger (DefinedSchema "writer"))

; ------------------------------------------------------------

; Low-level command: change a nick.
(Trigger (SetValue (NameNode "IRC chat object") (Predicate "*-write-*")
	(List (Concept "NICK dorkbot"))))

; Low-level command: list all channels w/ channel info
(Trigger (SetValue (NameNode "IRC chat object") (Predicate "*-write-*")
	(List (Concept "LIST"))))

; Low-level command: list details about one channel
(Trigger (SetValue (NameNode "IRC chat object") (Predicate "*-write-*")
	(List (Concept "LIST #opencog"))))

; Low-level command: all channels w/nicks in the channel
(Trigger (SetValue (NameNode "IRC chat object") (Predicate "*-write-*")
	(List (Concept "NAMES"))))

; Low-level command: nicks of users in a channel
(Trigger (SetValue (NameNode "IRC chat object") (Predicate "*-write-*")
	(List (Concept "NAMES #opencog"))))

; Low-level command: set channel topic.
(Trigger (SetValue (NameNode "IRC chat object") (Predicate "*-write-*")
	(List (Concept "TOPIC #opencog Who's an operator now"))))

(Trigger (SetValue (NameNode "IRC chat object") (Predicate "*-write-*")
	(List (Concept "HELP"))))

; HELP is one of the IRC server commands.

; --------------------------------------------------------
; Close the connection, and exit.
(Trigger
	(SetValue (NameNode "IRC chat object") (Predicate "*-close-*")))

; --------------------------------------------------------
; The End! That's All, Folks!
