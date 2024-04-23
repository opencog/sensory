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
(use-modules (opencog) (opencog exec) (opencog sensory))

; Open connection to an IRC server
(define irc-stream
   (cog-execute!
      (Open
         (Type 'IRChatStream)
         (SensoryNode "irc://echobot@irc.libera.chat:6667"))))

