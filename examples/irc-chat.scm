;
; irc-chat.scm -- IRC chat demo
;
; Demo opening channel to IRC and exchanging messages on it.
;
(use-modules (opencog) (opencog exec) (opencog sensory))

; --------------------------------------------------------
; Basic demo: Open a file for reading, at a fixed absolute location
; in the filesystem. Executing the FileNode will return a text
; stream value.
(define txt-stream
	(cog-execute!
		(Open
			(Type 'IRChatStream)
			(SensoryNode "irc://libera.chat:6667/opencog"))))

; Repeated references to the stream will return single lines from
; the file.
txt-stream

; --------------------------------------------------------
; The End! That's All, Folks!
