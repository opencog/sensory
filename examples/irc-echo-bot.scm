;
; irc-echo-bot.scm -- Demo of simple Atomese chatbot.
;
; Demo replicates some of the earliest, simplest and most primitive IRC
; bots. What's different here is that this one is written in Atomese.
; The goal is not to create "yet another bot", but to understand how an
; actual autonomous agent might interact in a simple world.
;
; See `irc-api.scm` for a tutorial on the interfaces used here.
; See `xterm-bridge.scm` for a simple demo of hooking input to output.
;
(use-modules (ice-9 threads))
(use-modules (srfi srfi-1))
(use-modules (opencog) (opencog exec) (opencog sensory))

; Open connection to an IRC server.

(define chatnode (IRChatNode "echobot"))
(cog-execute!
	(SetValue chatnode (Predicate "*-open-*")
	(Concept "irc://echobot@irc.libera.chat:6667")))

; Build a pair of read and Write accessors for the above location.
; Using the StreamValueOf automatically dereferences the stream for us.
; Using the naked ValueOf gives direct (read and write) access.
(define bot-read (StreamValueOf chatnode (Predicate "*-read-*")))
(define bot-raw (ValueOf chatnode (Predicate "*-read-*")))

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
		(LinkSignature (Type 'LinkValue) bot-read)))

(define private-echo
	(SetValue chatnode (Predicate "*-write-*") make-private-reply))

; Try it, once
(cog-execute! private-echo)

; The above works, but is "dangerous": if the bot is placed on a public
; channel, it will echo every post on that channel, back to the user who
; sent it. This will get annoying, fast, and get the bot kicked. Some
; sort of more restrained ehavior is needed.
;
; In a pure agential design, the bot might perceive the kick, and
; perhaps might perceive a kick as painful, and use that as a
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

; To accomplish this, a sequence of increasingly complex pipelines
; are developed below.
; -----------------
; Part one: Some generic boilerplate that all pipelines will use.

; Create a message-processiong rule. Accepts any input consisting
; of a LinkValue holding an IRC message, extracts the three parts,
; and then inserts the list of actoms in CONCLUSION. This is just
; sugar to avoid lots of cut-n-paste.
(define (make-msg-rule CONCLUSION)
	(Rule
		(VariableList
			(Variable "$from") (Variable "$to") (Variable "$msg"))
		(LinkSignature (Type 'LinkValue)
			(Variable "$from") (Variable "$to") (Variable "$msg"))
		(LinkSignature (Type 'LinkValue)
			CONCLUSION)))

; Convenience wrapper
(define (make-applier CONCLUSION)
	(Filter
		(make-msg-rule CONCLUSION)
		bot-raw))

; Convenience wrapper. Reads from IRC, extracts message, rewrites
; it into CONCLUSION, writes out to IRC.
(define (make-echoer CONCLUSION)
	(WriteLink bot-raw (make-applier CONCLUSION)))

; Most of the demos below need the bot to know it's own name.
(cog-set-value! (Anchor "IRC Bot")
	(Predicate "bot-name") (StringValue "echobot"))

; --------
; Below are a collection of examples. Some of these need the bot
; to sit on some public channel, in order to work.

; Join a channel.
(cog-execute! (Write bot-raw (List (Concept "JOIN #opencog"))))

; Leave a channel.
(cog-execute! (Write bot-raw (List (Concept "PART #opencog"))))

; --------
; Example: Show message
(define show (list (Variable "$from") (Variable "$to") (Variable "$msg")))
(cog-execute! (make-applier show))

; --------
; Example: No-op. Do nothing.
(define null-reply (list))
(cog-execute! (make-applier null-reply))

; --------
; Example: Break down message into parts.

; Is it a public or private message?
; It is private if (Variable "$to") is the name of the bot.
(define is-pub?
	(Cond
		(Equal (Variable "$to")
			(ValueOf (Anchor "IRC Bot") (Predicate "bot-name")))
		(Item "private message")
		(Item "public message")))
(cog-execute! (make-applier is-pub?))

; Create a private reply to the sender, printing mssage diagnostics.
(define id-reply
	(list (Item "PRIVMSG") (Variable "$from")
	(Item "Message to ")
	(Variable "$to")
	(Item " is a ")
	is-pub?
	(Item " from ")
	(Variable "$from")
	(Item ": ")
	(Variable "$msg")))
(cog-execute! (make-echoer id-reply))

; --------
; Example: Is the bot being called out?
; A "callout" is a string that starts with the botname, followed
; by a colon. This is an IRC convention, and nothing more.

(define is-callout?
	(Cond
		(Equal
			(ElementOf (Number 0) (Variable "$msg"))
			(ValueOf (Anchor "IRC Bot") (Predicate "bot-name")))
		(Item "calls out the bot")
		(Item "is just a message")))

(define callout-reply
	(list (Item "PRIVMSG") (Variable "$from")
	(Item "Message to ")
	(Variable "$to")
	(Item " is a ")
	is-pub?
	(Item " from ")
	(Variable "$from")
	(Item "that ")
	is-callout?
	(Item ": ")
	(Variable "$msg")))
(cog-execute! (make-echoer callout-reply))

; --------
; Example: Reply privately to all messages on a private channel,
; and only those messages on a public channel that call out the bot.

(define private-reply-to-callout
	(Cond
		(Or
			; Is this a private message?
			(Equal
				(Variable "$to")
				(ValueOf (Anchor "IRC Bot") (Predicate "bot-name")))
			; Is this a public callout?
			(Equal
				(ElementOf (Number 0) (Variable "$msg"))
				(ValueOf (Anchor "IRC Bot") (Predicate "bot-name"))))

		; Always reply privately.
		(LinkSignature
			(Type 'LinkValue)
				(Item "PRIVMSG") (Variable "$from")
				(Item "It seems that you said: ")
				(Variable "$msg"))
		(LinkSignature (Type 'LinkValue))))

(cog-execute! (make-echoer private-reply-to-callout))

; --------
; Example: Reply privately to all messages on a private channel,
; and publicly to callout messages on a public channel.

(define reply-to-callout
	(Cond
		; Is this a private message?
		(Equal
			(Variable "$to")
			(ValueOf (Anchor "IRC Bot") (Predicate "bot-name")))

		; Always reply privately.
		(LinkSignature
			(Type 'LinkValue)
				(Item "PRIVMSG") (Variable "$from")
				(Item "It seems that you said: ")
				(Variable "$msg"))

		; Reply only if called out.
		(Equal
			(ElementOf (Number 0) (Variable "$msg"))
			(ValueOf (Anchor "IRC Bot") (Predicate "bot-name")))

		; Reply politely. Note target is $to, not $from
		(LinkSignature
			(Type 'LinkValue)
				(Item "PRIVMSG") (Variable "$to")
				(Item "Did ")
				(Variable "$from")
				(Item " say \"")
				(Variable "$msg")
				(Item "\"?"))

		; Ignore
		(VoidValue)))

(cog-execute! (make-echoer reply-to-callout))

; --------
; Example: listen to everything, and write it to a file. This requires
; opening a log-file.

; XXX unfinished...

; -------------------------------------------------------

; Hack for long-running streams. This part of the system is not yet
; fully designed. Details will change.
;
; The `OutputStream::do_write_out()` method does enter an inf loop,
; copying source to sink for as long as the source stays open. For
; whatever reason, that is not happening in this IRC demo, probably
; because all the filters obscure that there is a stream at the far
; end. So this needs work.
;
; So, for now, hack around this. Create an inf loop here, and run it
; in it's own thread. FYI, Tail-recursive loops can also be done in
; Atomese; the below is done in pure scheme.

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
;(define thread-id (call-with-new-thread
;	(lambda () (inf-loop private-echo))))

; Start an inf loop with the friendly echo handler
(define thread-id (call-with-new-thread
	(lambda () (inf-loop (make-echoer reply-to-callout)))))

(exit-loop)

; The End. That's all, folks!
; -------------------------------------------------------
