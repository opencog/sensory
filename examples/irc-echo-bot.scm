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

; The chatbot will be called "echobot" on IRC.
; The NameNode is just a handy handle that we can use in the
; Atomese code, so that the chatbot name is not hard-coded
; further on in this demo.
(PipeLink
	(NameNode "IRC chat object")
	(IRChatNode "echobot"))

; Open connection to an IRC server.
(Trigger
	(SetValue (NameNode "IRC chat object") (Predicate "*-open-*")
	(Concept "irc://echobot@irc.libera.chat:6667")))

; The StreamValueOf automatically unqueues one message from the stream.
(Pipe
	(Name "IRC read")
	(StreamValueOf (NameNode "IRC chat object") (Predicate "*-stream-*")))

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
		(Name "IRC chat object") (Predicate "*-write-*")
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
; sort of more restrained behavior is needed.
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
; So how can we reply on a public channel?
;
; As noted above, DO NOT join a public channel while the private-echo
; agent is running (in the inf-loop, at bottom). It's incessant replies
; will get it kicked. So we want a message processing pipeline that is
; aware of being on a public channel, and replies only when spoken to.

; To accomplish good  citizen behavor on public channels, a sequence of
; increasingly complex pipelines are developed below.
; -----------------
; Part one: Define some generic boilerplate that all pipelines will use.

; Define a convenience utility. This is a Filter that will pull from
; IRC, and apply a RuleLink to each message. The RuleLink will unwrap
; each message into three variables, and then pass the variables to the
; "conclusion", which is some rewrite template on these three variables.
;
(Define
	(DefinedSchema "message rewriter")
	(Lambda
		(Variable "rewrite of message")
		(Filter
			(Rule
				(VariableList
					(Variable "$from") (Variable "$to") (Variable "$msg"))
				(LinkSignature (Type 'LinkValue)
					(Variable "$from") (Variable "$to") (Variable "$msg"))
				(Variable "rewrite of message"))
		(Name "IRC read"))))

; --------
; Example of using the above: Show a message
(Define
	(DefinedSchema "display mesg")
	(LinkSignature (Type 'LinkValue)
		(Concept "Get a message from: ")
		(Variable "$from")
		(Concept "To: ")
		(Variable "$to")
		(Concept "The message: ")
		(Variable "$msg")))

; This places the rewrite (DefinedSchema "display mesg") into
; the (DefinedSchema "message rewriter") and then runs the combo.
;
; This will hang, until you say something to the bot. Once you do.
; it will show what was received.
(Trigger
	(Put
		(DefinedSchema "message rewriter")
		(DefinedSchema "display mesg")))

; --------
; It's time to join a public channel; the remaining demos show how to
; use the bot on a pubic channel.

; Join a channel.
(Trigger (SetValue (Name "IRC chat object") (Predicate "*-write-*")
	(List (Concept "JOIN #opencog"))))

; Leave a channel.
(Trigger (SetValue (Name "IRC chat object") (Predicate "*-write-*")
	(List (Concept "PART #opencog"))))

; --------
; Example: Determine if the bot is being addressed on public channel,
; or if it is in a private chat.

; Utility: Return the bot name as a StringValue
(PipeLink
	(NameNode "IRC botname")
	(LinkSignature (Type 'StringValue) (Name "IRC chat object")))

; Is it a public or private message?
; It is private if (Variable "$to") is the name of the bot.
(Define
	(DefinedSchema "is public?")
	(Cond
		(Equal (Variable "$to") (Name "IRC botname"))
		(Item "private message")
		(Item "public message")))

; As before, this will hang until a message is sent to the bot.
(Trigger
	(Put
		(DefinedSchema "message rewriter")
		(DefinedSchema "is public?")))

; --------
; Define a message responder. Much like the previously defined
; (DefinedSchema "message rewriter"), this is a generic function
; that will push the rewrite back out to IRC. Whether the response
; shows as a private message, or a public message depends on the
; specific rewrite. Both will be demoed below.
;
(Define
	(DefinedSchema "responder")
	(Lambda
		(Variable "rewrite of message")
		(SetValue (Name "IRC chat object") (Predicate "*-write-*")
			(LinkSignature (Type 'LinkValue)
				(Put
					(DefinedSchema "message rewriter")
					(Variable "rewrite of message"))))))

; Create a private reply to the sender, printing message diagnostics.
(Define
	(DefinedSchema "private reply")
	(LinkSignature (Type 'LinkValue)
		(Item "PRIVMSG")
		(Variable "$from")
		(Item "Message to ")
		(Variable "$to")
		(Item " is a ")
		(DefinedSchema "is public?")
		(Item " from ")
		(Variable "$from")
		(Item ": ")
		(Variable "$msg")))

; Run the above. This requires two triggers, an artifact of using
; two nested PutLinks instead of one. The first trigger assembles
; the message handler; the second runs it.
(Trigger (Trigger
	(Put
		(DefinedSchema "responder")
		(DefinedSchema "private reply"))))


; --------
; Example: Is the bot being called out?
; A "callout" is a string that starts with the botname, followed
; by a colon. This is an IRC convention, and nothing more.

(Define
	(DefinedSchema "is callout?")
	(Cond
		(Equal
			(ElementOf (Number 0) (Variable "$msg"))
			(Name "IRC botname"))
		(Item "calls out the bot")
		(Item "is just a message")))

(Define
	(DefinedSchema "callout reply")
	(LinkSignature (Type 'LinkValue)
		(Item "PRIVMSG") (Variable "$from")
		(Item "Message to ")
		(Variable "$to")
		(Item " is a ")
		(DefinedSchema "is public?")
		(Item " from ")
		(Variable "$from")
		(Item " that ")
		(DefinedSchema "is callout?")
		(Item ": ")
		(Variable "$msg")))

(Trigger (Trigger
	(Put
		(DefinedSchema "responder")
		(DefinedSchema "callout reply"))))


; --------
; Example: Reply privately to all messages on a private channel,
; and only those messages on a public channel that call out the bot.

(define private-reply-to-callout
	(Cond
		(Or
			; Is this a private message?
			(Equal
				(Variable "$to")
				(Name "IRC botname"))
			; Is this a public callout?
			(Equal
				(ElementOf (Number 0) (Variable "$msg"))
				(Name "IRC botname")))

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
			(Name "IRC botname"))

		; Always reply privately.
		(LinkSignature
			(Type 'LinkValue)
				(Item "PRIVMSG") (Variable "$from")
				(Item "It seems that you said: ")
				(Variable "$msg"))

		; Reply only if called out.
		(Equal
			(ElementOf (Number 0) (Variable "$msg"))
			(Name "IRC botname"))

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
