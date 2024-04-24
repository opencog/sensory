;
; irc-echo-bot.scm -- Demo of simple Atomese chatbot.
;
; Demo replicates some of the earliest, simplest and most primitive IRC
; bots. What's different here is that this one is written in Atomese.
; The goal is not to create "yet another bot", but to understand how an
; actual autonomous agent might interact in a simple world.
;
; See `irc-api.scm` for a tutorial on the interfaces used here.
;
(use-modules (ice-9 threads))
(use-modules (srfi srfi-1))
(use-modules (opencog) (opencog exec) (opencog sensory))

; Open connection to an IRC server, and place the resulting stream
; where we can find it again.
(cog-execute!
	(SetValue
		(Anchor "IRC Bot") (Predicate "echo")
		(Open (Type 'IRChatStream)
			(SensoryNode "irc://echobot@irc.libera.chat:6667"))))

; Read and Write accessor for the above location.
; Using the StreamValueOf automatically dereferences the stream for us.
; Using the naked ValueOf gives direct access.
(define bot-read (StreamValueOf (Anchor "IRC Bot") (Predicate "echo")))
(define bot-raw (ValueOf (Anchor "IRC Bot") (Predicate "echo")))

; An alternate (better?) design would be to not run the execute!
; until later, when we actually need it. For now, we punt.

; Individual messages can be read like so:
(cog-execute! bot-read)

; Join a channel.  This is a hack, for demo/testing. A real
; agent would have freedom to wander channel-space.
(cog-execute! (Write bot-raw (List (Concept "JOIN #opencog"))))

; Leave, like so:
; (cog-execute! (Write bot-raw (List (Concept "PART #opencog"))))

; -------------------------------------------------------
; Set up the basic ping filter.
;
; A single message can be read by saying (cog-execute! bot-read)
;
; Messages come in two types: private messages, and channel messages.
; Private messages have the form:
;    (LinkValue
;       (StringValue "linas")
;       (StringValue "echobot")
;       (StringValue "bunch o text")))
;
; Public messages have the form:
;    (LinkValue
;       (StringValue "linas")
;       (StringValue "#opencog")
;       (StringValue "bunch o text")))
;
; The difference between the two is the target, in the second location.
;
; The filter below, when executed, will pull in a single message,
; pattern match it, and write a private reply to the sender.
; More correctly, executing `make-private-reply` will get one message,
; and rewrite it. Executing the private-echo will read-modify-write.

(define make-private-reply
	(Filter
		(Rule
			(VariableList
				(Variable "$from") (Variable "$to") (Variable "$msg"))
			(LinkSignature (Type 'LinkValue)
				(Variable "$from") (Variable "$to") (Variable "$msg"))
			(LinkSignature (Type 'LinkValue)
				(Item "PRIVMSG")
				(Variable "$from")
				(Item "you said: ")
				(Variable "$msg")))
		bot-raw))

(define private-echo (Write bot-raw make-private-reply))

; Try it, once
(cog-execute! private-echo)

; -------------------------------------------------------

; Create an infinite loop. This will block if there is nothing to read.
(define do-exit-loop #f)
(define (inf-loop)
	(cog-execute! private-echo)
	(if (not do-exit-loop) (inf-loop)))

(define thread-id (call-with-new-thread inf-loop))
(define (exit-loop)
	(set! do-exit-loop #t)
	(join-thread thread-id)
	(format #t "Exited main loop\n")
	(set! do-exit-loop #f))

; The End. That's all, folks!
; -------------------------------------------------------
