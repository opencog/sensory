;
; xterm-bridge.scm -- copying between a pair of xterms.
;
; Demo showing how to set up a pair of pipelines between two xterms.
; Text typed into one appears in the other, and vice-versa.
;
; The bridge is constructed manually, without any automation. It is a
; two-lane bridge, as each xterm can be both a source and also a sink
; for text, and so there will be two connections, input to output.
;
; The eventual goal, in a later demo, is to automatically construct
; this bridge (these kinds of bridges), by linking together all open
; connectors on all device descriptions. This demo shows one of the
; simplest examples of what such running links could look like.
;
; For a more basic review of how to read and write from inputs and
; outputs, review the `xterm-io.scm` demo first.
;
; Minor warning: something with xterm might be slightly flakey, in
; that sometimes, text is copied twice. This is not a bug in the
; Atomese code, but some weirdness with xterm and pseudo-terminal
; /dev/pts pipes. Not worth debugging, since this is not a critical
; component. So, just FYI. See `fgets_unlocked` in the source for more.

(use-modules (opencog) (opencog exec) (opencog sensory))

; --------------------------------------------------------
; Create a pair of xterms; place them where they can be found.

(define axterm (TerminalNode "term A"))
(define bxterm (TerminalNode "term B"))

(cog-set-value! axterm (Predicate "*-open-*") (VoidValue))
(cog-set-value! bxterm (Predicate "*-open-*") (VoidValue))

; Create two copiers. The

; Executing each of these once will create
; a pipe that will flow text from one terminal to the other, for
; as long as the terminals are running. The flow is done by an
; infinite loop in the C++ method OutputStream::do_write_out().
; Ths method, when given a stream to read, will keep pulling data
; from that stream until the stream closes. Since the inf loop runs
; in the current thread, calling cog-execute! on these will not
; return until the streams close. Thus, to keep the streams running,
; and have the current thread availabe for other work, it is best
; to execute these each in their own thread.
;
; Sometimes, when switching from one terminal to the other, you
; might see the echoed text duplicated. This is a bug. It's due to
; some weird fgets-threading-locking bug, see TerminalStream.cc
; for details. Low priority, so not fixed.
(define copy-b-to-a
	(SetValue axterm (Predicate "*-write-*")
		(ValueOf bxterm (Predicate "*-read-*"))))

(define copy-a-to-b
	(SetValue bxterm (Predicate "*-write-*")
		(ValueOf axterm (Predicate "*-read-*"))))

(call-with-new-thread (lambda () (cog-execute! copy-b-to-a)))
(call-with-new-thread (lambda () (cog-execute! copy-a-to-b)))

; --------------------------------------------------------
; The End! That's All, Folks!
