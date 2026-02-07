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
(use-modules (opencog) (opencog sensory))

; Open connection to an IRC server.

(define (Name "IRC botname") (IRChatNode "echobot"))
(PipeLink
	(NameNode "IRC botname")
	(IRChatNode "echobot"))

(Trigger
	(SetValue (NameNode "IRC botname") (Predicate "*-open-*")
	(Concept "irc://echobot@irc.libera.chat:6667")))

; The StreamValueOf automatically unqueues one message from the stream.
(Pipe
	(Name "IRC read")
	(StreamValueOf (NameNode "IRC botname") (Predicate "*-stream-*")))

; Individual messages can be read like so:
(Trigger (Name "IRC read"))
(Trigger (Name "IRC read"))

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
; More correctly, there is a named pipe, the output end of which is
; given the name `(Name "private echo")`. When this endpoint is
; pulled from, it will cause a filter to run. This filter will pull
; from it's input, which happens to be another named pipe, having
; the name `(Name "IRC read")`.  And when that pipe is pulled on,
; it will go off the IRC bot and get one line, per  pull.

(Pipe
	(Name "private echo")
	(SetValue
		(Name "IRC botname") (Predicate "*-write-*")
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
			(Name "IRC read"))))

; Try it, once
(Trigger (Name "private echo"))

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

; Convenience wrapper.
; Note that executing this will hang, waiting on input, and so you
; have to say something to the bot before this returns.
(define (make-applier CONCLUSION)
	(Filter
		(make-msg-rule CONCLUSION)
		(Name "IRC read")))

; Convenience wrapper. Reads from IRC, extracts message, rewrites
; it into CONCLUSION, writes out to IRC.
(define (make-echoer CONCLUSION)
	(SetValue (Name "IRC botname") (Predicate "*-write-*") (make-applier CONCLUSION)))

; Most of the demos below need the bot to know it's own name.
(cog-set-value! (Anchor "IRC Bot")
	(Predicate "bot-name") (StringValue "echobot"))

; --------
; Below are a collection of examples. Some of these need the bot
; to sit on some public channel, in order to work.

; Join a channel.
(Trigger (SetValue (Name "IRC botname") (Predicate "*-write-*")
	(List (Concept "JOIN #opencog"))))

; Leave a channel.
(Trigger (SetValue (Name "IRC botname") (Predicate "*-write-*")
	(List (Concept "PART #opencog"))))

; --------
; Example: Show message
; This hangs, until you say something to the bot.
(define show (list (Variable "$from") (Variable "$to") (Variable "$msg")))
(Trigger (make-applier show))

; --------
; Example: No-op. Do nothing.
(define null-reply (list))
(Trigger (make-applier null-reply))

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
(Trigger (make-applier is-pub?))

; Create a private reply to the sender, printing message diagnostics.
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
(Trigger (make-echoer id-reply))

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
	(Item " that ")
	is-callout?
	(Item ": ")
	(Variable "$msg")))
(Trigger (make-echoer callout-reply))

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

(Trigger (make-echoer private-reply-to-callout))

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

(Trigger (make-echoer reply-to-callout))

; --------
; Example: listen to everything, and write it to a file. This requires
; opening a log-file, and then piping chat content to that file.

(define irc-log-file (TextFile "file:///tmp/irc-chatlog.txt"))
(Trigger
	(SetValue irc-log-file (Predicate "*-open-*") (Type 'StringValue)))

(cog-set-value! irc-log-file (Predicate "*-write-*")
	(StringValue "Start of the log file\n"))

;;; (Trigger
;;;	(SetValue irc-log-file (Predicate "*-close-*") (Type 'StringValue)))

;; This logger will pull everything from IRC (that the bit can hear)
;; and will write it to the logfile. It will do this forever, i.e.
;; it's an infini9te loop, so it needs to be run in it's own thread.
;;
;; It works, except that there's a small problem: There are no newlines
;; at the end of messages, and the send & recipient are not demarcated.
;; So everything written to the logfile will be an ever-expanding blob.
(define logger
	(SetValue irc-log-file (Predicate "*-write-*")
		(ValueOf (Name "IRC botname") (Predicate "*-stream-*"))))

; Don't do this, unless you want the blob, explained above.
; (Trigger (ExecuteThreaded logger))

; Instead, write a message formatter. This picks apart the message,
; wraps it in some delimiters, and prints to the logfile. Much nicer!
(define format-for-logger
	(Filter
		(Rule
			(VariableList
				(Variable "$from") (Variable "$to") (Variable "$msg"))
			(LinkSignature (Type 'LinkValue)
				(Variable "$from") (Variable "$to") (Variable "$msg"))
			(LinkSignature (Type 'LinkValue)
				(Item "MSG From: ")
				(Variable "$from")
				(Item " To: ")
				(Variable "$to")
				(Item " Message: ")
				(Variable "$msg")
				(Item "\n")))
		(StreamValueOf (Name "IRC botname") (Predicate "*-stream-*"))))

; The formatter runs fine: Try it, one line at a time
(define one-at-a-time-logger
	(SetValue irc-log-file (Predicate "*-write-*") format-for-logger))
(Trigger one-at-a-time-logger)

; But we don't want a one-a-a-time interface; we want this to run
; indefinitely.  For that, we need a promise, that can pull items
; through the filter. (The filter itself cannot push.)
; The FutureStream can pull items through the Filter. So wrap the
; filter in that. When this is attached to the file writer, it
; will run forever. The infinite loop doing this is located in
; StreamNode::write() method.
(define logger
	(SetValue irc-log-file (Predicate "*-write-*")
		(CollectionOf (TypeNode 'FutureStream) (OrderedLink format-for-logger))))

; Run the logger in it's own thread.
(Trigger (ExecuteThreaded logger))

; The End. That's all, folks!
; -------------------------------------------------------
