;
; ollama-bot.scm -- IRC chatbot powered by Ollama LLM
;
; Demo combining the IRC interface with the Ollama interface. The bot
; connects to IRC, joins #opencog, and responds to messages by passing
; them through Ollama and relaying the LLM's response.
;
; See `irc-echo-bot.scm` for the IRC pipeline patterns used here.
; See `ollama-api.scm` for the basic Ollama interface.
;
; HINT: The demo below makes heavy use of NameNodes and DefinedSchemas.
; These can only be defined once, ever. Attempting a redefinition will
; throw an exception. So if you want to experiment, you will need to
; erase these. You can do this with (DeleteRecursive (Name "whatever"))
; or (DeleteRecursive (DefinedSchemaNode "stuffs")).
;
(use-modules (opencog) (opencog sensory))

; --------------------------------------------------------
; Part 1: Open connections to both IRC and Ollama.

; The chatbot will be called "ollamabot" on IRC.
(PipeLink
	(NameNode "IRC chat object")
	(IRChatNode "ollamabot"))

; Open connection to IRC.
(Trigger
	(SetValue (NameNode "IRC chat object") (Predicate "*-open-*")
		(Item "irc://ollamabot@irc.libera.chat:6667")))

; Open connection to Ollama. Adjust the model name as needed.
(Trigger
	(SetValue (OllamaNode "my-llm") (Predicate "*-open-*")
		(Item "ollama://localhost:11434/qwen3:8b")))

; Set up the IRC stream reader.
(Pipe
	(Name "IRC read")
	(StreamValueOf (NameNode "IRC chat object") (Predicate "*-stream-*")))

; The IRC server will send us exacly two startup messages.
; Clear these out of the way; we don't want to sent these to Ollama.
(Trigger (Name "IRC read"))
(Trigger (Name "IRC read"))

; Join a channel.
(Trigger (SetValue (Name "IRC chat object") (Predicate "*-write-*")
	(List (Concept "JOIN #opencog"))))

; --------------------------------------------------------
; Part 2: Set up the message processing pipeline.
; This is a simplified version of what is in irc-echo-bot.scm.

; Generic responder: a Lambda that takes a rewrite template, applies
; it to the next IRC message via the message rewriter, and writes
; the result back to IRC.
(Define
	(DefinedSchema "responder")
	(Lambda
		(Variable "$rewrite")
			(LinkSignature (Type 'LinkValue)
		(Filter
			(Rule
				(VariableList
					(Variable "$from") (Variable "$to") (Variable "$msg"))
				(LinkSignature (Type 'LinkValue)
					(Variable "$from") (Variable "$to") (Variable "$msg"))
				(SetValue (Name "IRC chat object") (Predicate "*-write-*")
					(Variable "$rewrite")))
			(Name "IRC read")))))

; Utility: return the bot name as a StringValue.
(PipeLink
	(NameNode "IRC botname")
	(LinkSignature (Type 'StringValue) (Name "IRC chat object")))

; --------------------------------------------------------
; Part 3: The Ollama-powered reply logic.
;
; The key trick:
;    (ValueOf (SetValue ollama *-write-* text) *-read-*)
;
; SetValue, when executed, writes to Ollama and returns the OllamaNode.
; ValueOf then reads from that same node, blocking until the LLM
; finishes generating. This chains the write-then-read in a single
; expression.
;
; IRC messages arrive as:
;    (LinkValue (StringValue "nick") (StringValue "#chan") (StringValue "text"))
;
; When a message starts with the bot name (a "callout"), the $msg part
; is a StringValue with two elements: ("botname" "the rest of text").
; ElementOf is used to extract the text portion.

(Define
	(DefinedSchema "ollama reply")
	(Cond
		; Is this a private message to the bot?
		(Equal (Variable "$to") (Name "IRC botname"))

		; Reply privately with Ollama's response.
		(LinkSignature (Type 'LinkValue)
			(Item "PRIVMSG") (Variable "$from")
			(ValueOf
				(SetValue (OllamaNode "my-llm") (Predicate "*-write-*")
					(Variable "$msg"))
				(Predicate "*-read-*")))

		; Is this a public callout? (e.g. "ollamabot: what is 2+2?")
		(Equal
			(ElementOf (Number 0) (Variable "$msg"))
			(Name "IRC botname"))

		; Reply on the channel with Ollama's response.
		; ElementOf extracts the text after the bot name.
		(LinkSignature (Type 'LinkValue)
			(Item "PRIVMSG") (Variable "$to")
			(ValueOf
				(SetValue (OllamaNode "my-llm") (Predicate "*-write-*")
					(ElementOf (Number 1) (Variable "$msg")))
				(Predicate "*-read-*")))

		; Not addressed to us: ignore.
		(Link)))

; --------------------------------------------------------
; Part 4: Run the bot.

; The bot handler: read one IRC message, process it, reply.
(Define
	(DefinedSchema "the ollama bot")
	(Trigger
		(Put
			(DefinedSchema "responder")
			(DefinedSchema "ollama reply"))))

; Try it once. This blocks until someone sends a message.
(Trigger (DefinedSchema "the ollama bot"))

; Run forever in a background thread.
(Trigger (ExecuteThreaded
	(DrainLink (DefinedSchema "the ollama bot"))))

; --------------------------------------------------------
; Cleanup (when done).
; (Trigger
;	(SetValue (NameNode "IRC chat object") (Predicate "*-close-*")))
; (SetValue (OllamaNode "my-llm") (Predicate "*-close-*"))

; --------------------------------------------------------
; The End! That's All, Folks!
