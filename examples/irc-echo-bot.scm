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

; Open connection to an IRC server.
(define irc-stream
   (cog-execute!
      (Open
         (Type 'IRChatStream)
         (SensoryNode "irc://echobot@irc.libera.chat:6667"))))

; Place the stream at a "well-known location".
(cog-set-value! (Anchor "IRC Bot") (Predicate "echo") irc-stream)
(define echobot (ValueOf (Anchor "IRC Bot") (Predicate "echo")))

; A better design for above would be a declarative (Open...)
; followed by an execute much later.  For now, we punt.

; Join a channel.  This is a hack, for demo/testing. A real
; agent would have freedeom to wander channel-space.
(cog-execute! (Write echobot (List (Concept "JOIN #opencog"))))

; -------------------------------------------------------

; Create an infinite loop. This will block if there is nothing to read.
(define do-exit-loop #f)
(define msg #f)
(define (inf-loop)
	(set! msg (car (cog-value->list (cog-execute! echobot))))
	(format #t "overheard ~A\n" msg)
	(if (not do-exit-loop) (inf-loop)))

(define thread-id (call-with-new-thread inf-loop))
(define (exit-loop)
	(set! do-exit-loop #t)
	(join-thread thread-id)
	(format #t "Exited main loop\n")
	(set! do-exit-loop #f))

; The above uses a lot of scheme to get things done, including the
; thread declaration. We should do this in Atomese, instead,


