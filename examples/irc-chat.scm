;
; irc-chat.scm -- IRC chat demo
;
; Demo opening channel to IRC and exchanging messages on it.
;
(use-modules (opencog) (opencog exec) (opencog sensory))

; --------------------------------------------------------
; Basic demo: Open a connection to an IRC node. The connection
; is given by a URL of the form
; irc://nick[:pass]@host[:port]/#channel
;
(define irc-stream
	(cog-execute!
		(Open
			(Type 'IRChatStream)
			(SensoryNode "irc://botty@irc.libera.chat:6667/#opencog"))))

; Repeated references to the stream will return single lines from
; the file.
irc-stream

; Write stuff too
(cog-set-value!
   (Anchor "IRC Bot") (Predicate "tester") irc-stream)

; Create a WriteLink
(define writer
   (WriteLink
      (ValueOf (Anchor "IRC Bot") (Predicate "tester"))
      (ValueOf (Anchor "Stuff to say") (Predicate "say key"))))

(cog-set-value!
	(Anchor "Stuff to say") (Predicate "say key")
	(StringValue "PRIVMSG" "#opencog" "I have something to say"))

; Write to the chat channel.
(cog-execute! writer)

; --------------------------------------------------------
; The End! That's All, Folks!
