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

; Open connection to an IRC server, and attach the stream to an Atom.
; This allows it to be located, when needed. If the Atom is "well
; known", then everyone can find it.
(cog-execute!
	(SetValue
		(Anchor "IRC Bot") (Predicate "echo")
		(Open (Type 'IRChatStream)
			(SensoryNode "irc://echobot@irc.libera.chat:6667"))))

; Build a pair of read and Write accessors for the above location.
; Using the StreamValueOf automatically dereferences the stream for us.
; Using the naked ValueOf gives direct (read and write) access.
(define bot-read (StreamValueOf (Anchor "IRC Bot") (Predicate "echo")))
(define bot-raw (ValueOf (Anchor "IRC Bot") (Predicate "echo")))

; Individual messages can be read like so:
(cog-execute! bot-read)

; An alternate design would be to finish setting everything up, before
; opening a connection. But we want to demo it's operation as we go
; along. Having a live connection is harmless.

; -------------------------------------------------------
; Set up the basic echo filter.
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
; and rewrite it; but that's it it doesn't send. The private-echo
; below will read-modify-write.

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

; The above works, but is "dangerous": if the bot is placed on a public
; channel, it will echo every post on that channel, back to the user who
; sent it. This will get annoying, fast, and get the bot kicked. Some
; sort of more restrained ehavior is needed.
;
; In a pure agential design, the bot might perceive the kick, and
; perhaps might percieve a kick as painful, and use that as a
; hint to modify it's behavior. However, using evolutionary pressure,
; where users kick annoying bots, is perhaps not a very good way of
; running agents.
;
; This "agential issue" is not solvable in the present context. The rest
; of this demo continues to focus on pipeline processing.

; -------------------------------------------------------
; How about a reply on a public channel?
;
; As noted above, DO NOT join a public channel while the private-echo
; agent is running (in the inf-loop, at bottom). It's incessant replies
; will get it kicked. So we want a message processing pipeline that is
; aware of being on a public channel, and replies only when spoken to.

; To accomplish this, we first need a tutorial on Atomese pipelines.
; So here's a sequence of tricks.
; -----------------
; Trick: calling scheme code (this works for python, too)

(use-modules (opencog) (opencog exec) (opencog sensory))
(define exo
	(ExecutionOutput
		(GroundedSchema "scm: foo")  ; the function
		(List (Concept "bar"))))     ; the arguments

; A scheme function that will be called. In general, these should
; return some Atom or Value.
(define (foo x)
	(format #t "I got ~A\n" x)
	(Concept "foo reply"))   ; Must return an Atom, else an error will

; Run it and see.
(cog-execute! exo)

; -----------------
; Trick:


(define foobar-echo
	(WriteLink bot-raw
		(Filter
			(Rule
				(VariableList
					(Variable "$from") (Variable "$to") (Variable "$msg"))
				(LinkSignature (Type 'LinkValue)
					(Variable "$from") (Variable "$to") (Variable "$msg"))
				(LinkSignature (Type 'LinkValue)
					(Item "PRIVMSG")
					(Variable "$from")
					(Item "Du, you said: ")
					(Variable "$msg")))
			bot-raw)))



; Join a channel.
(cog-execute! (Write bot-raw (List (Concept "JOIN #opencog"))))

; Leave, like so:
; (cog-execute! (Write bot-raw (List (Concept "PART #opencog"))))

; -------------------------------------------------------

; Create an infinite loop. The AGENT must be an ececutable Atom;
; presumably, it is some Atom that interacts with IRC in some way.
; As long as AGENT exits every now and then, the loop can be
; interruped, as shown below.
;
; XXX Tail-recursive loops can also be done in Atomese, and perhaps
; the below should be replaced by a pure-Atomese version. XXX FIXME.

(define do-exit-loop #f)
(define (inf-loop AGENT)
	(cog-execute! AGENT)
	(if (not do-exit-loop) (inf-loop AGENT)))

(define thread-id #f)
(define (exit-loop)
	(set! do-exit-loop #t)
	(join-thread thread-id)
	(format #t "Exited main loop\n")
	(set! do-exit-loop #f))

; Start an inf loop with the private-echo handler.
(define thread-id (call-with-new-thread
	(lambda () (inf-loop private-echo))))

; (exit-loop)

; The End. That's all, folks!
; -------------------------------------------------------
