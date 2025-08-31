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
(define chatnode (IRChatNode "blondie"))
(cog-set-value! chatnode (Predicate "*-open-*")
	(StringValue "irc://blondie@irc.libera.chat:6667"))

; Repeated references to the stream will return single lines from
; the file.
(cog-execute! (ValueOf chatnode (Predicate "*-read-*")))

; Join an IRC channel
(cog-set-value! chatnode (Predicate "*-write-*")
	(List (Concept "JOIN") (Concept "#opencog")))

; Say something on that channel
(cog-set-value! chatnode (Predicate "*-write-*")
	(List (Concept "PRIVMSG") (Concept "#opencog")
		(Concept "Here's a bunch of words I want to say")))

; ------------------------------------------------------------
; The above explicitly provides IRC commands and text as Atoms.
; For agents, it is more convenient to flow these as a stream.
; For this purpose, create a WriteLink that, when executed, will
; copy from the input stream to the IRC stream.
(define writer
	(SetValue
		chatnode (Predicate "*-write-*")
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

; Low-level command: change a nick.
(cog-set-value! chatnode (Predicate "*-write-*")
	(List (Concept "NICK dorkbot")))

; Low-level command: list all channels w/ channel info
(cog-set-value! chatnode (Predicate "*-write-*")
	(List (Concept "LIST")))

; Low-level command: list details about one channel
(cog-set-value! chatnode (Predicate "*-write-*")
	(List (Concept "LIST #opencog")))

; Low-level command: all channels w/nicks in the channel
(cog-set-value! chatnode (Predicate "*-write-*")
	(List (Concept "NAMES")))

; Low-level command: nicks of users in a channel
(cog-set-value! chatnode (Predicate "*-write-*")
	(List (Concept "NAMES #opencog")))

; Low-level command: set channel topic.
(cog-set-value! chatnode (Predicate "*-write-*")
	(List (Concept "TOPIC #opencog Who's an operator now")))

(cog-set-value! chatnode (Predicate "*-write-*")
	(List (Concept "HELP")))

; HELP is one of the IRC server commands.

; --------------------------------------------------------
; The End! That's All, Folks!
