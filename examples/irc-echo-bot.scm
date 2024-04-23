;
; irc-echo-bot.scm -- Demo of simple Atomese chatbot.
;
; Demo replicates some of the earliest, simplest and most primitive IRC
; bots. What's different here is that this one is written in Atomese.
; The goal is not to create "yet another bot", but to understand how an
; actual autonomous agent might interact in a simple world.
;
; See `irc-api.scm` for a totorial on the interfaces used here.
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
(define bot-write (ValueOf (Anchor "IRC Bot") (Predicate "echo")))

; An alternate (better?) design would be to not run the execute!
; until later, when we actually need it. For now, we punt.

; Join a channel.  This is a hack, for demo/testing. A real
; agent would have freedeom to wander channel-space.
(cog-execute! (Write bot-write (List (Concept "JOIN #opencog"))))

; -------------------------------------------------------
; Set up stream reading
; The current message can be read (using guile) as
;    (define msg (cog-value-ref (cog-execute! bot-read) 2)))
;
; We want the Atomese equivalent for the above.
;
(define ext
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
		(LinkSignature (Type 'LinkValue) bot-read)))

(cog-execute! ext)

; -------------------------------------------------------
; Set up stream writing.

; The place where commands will be streamed.
(define cmd-source (ValueOf (Anchor "IRC Bot") (Predicate "cmd")))

; The writer, that will copy from the cmd-source to the bot,
; whenever it is executed.
(define writer (Write bot-write cmd-source))

; Initial greeting
(cog-set-value! (Anchor "IRC Bot") (Predicate "cmd")
	(StringValue "PRIVMSG" "linas" "deadbeef"))

(cog-set-value! (Anchor "IRC Bot") (Predicate "cmd")
	(LinkValue
		(Item "PRIVMSG") (StringValue "linas")
			(StringValue "deadbeef text")))

; -------------------------------------------------------
; XXX Use SetValue instead...
(define (set-msg msg)
	(cog-set-value! (Anchor "IRC Bot") (Predicate "cmd")
		(StringValue "PRIVMSG" "linas" msg)))

; Create an infinite loop. This will block if there is nothing to read.
(define do-exit-loop #f)
(define (inf-loop)
	; Messy gunk to deref the value instead of letting
	; guile hang us.
	(define msg (car (cog-value->list (cog-execute! echobot))))
	(format #t "overheard ~A\n" msg)

	(define txt (cog-value-ref msg 2))
	(set-msg (string-append "you said " txt))
	(cog-execute! writer)
	(if (not do-exit-loop) (inf-loop)))

(define thread-id (call-with-new-thread inf-loop))
(define (exit-loop)
	(set! do-exit-loop #t)
	(join-thread thread-id)
	(format #t "Exited main loop\n")
	(set! do-exit-loop #f))

; The above uses a lot of scheme to get things done, including the
; thread declaration. We should do this in Atomese, instead,


; The End. That's all, folks!
; -------------------------------------------------------
