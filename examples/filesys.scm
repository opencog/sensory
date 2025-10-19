;
; filesys.scm -- filesystem demo
;
; Demo the filesystem navigation API. This demos the "already known"
; API for the filesystem object. That is, it is assumed that we
; "magically know" the API already (e.g. by reading the documentation)
; and so have skipped over the discovery step. See the discovery
; demo at `introspection.scm` for how to discover an object API.
;
(use-modules (opencog) (opencog exec) (opencog sensory))

; -----------------------------------------------------------
; Preliminary setup.

; Open the filesystem node.
(define fsnode (FileSysNode "file:///tmp"))
(cog-set-value! fsnode (Predicate "*-open-*") (Type 'StringValue))

; In principle, the agent should do the above after discovery.
; For now, for this demo, just hard-code it, as above.

; -----------------------------------------------------------
; The filesystem object has an interleaved read-write API.
; A command, such as `pwd` or `cd` or `ls` is sent using the
; `*-write-*` message, followed up by a `*-read-*` message to get the
; command results. These are in strict pairs, with the command results
; being placed on a QueueValue. This design means that the reads and
; writes can be done from different threads. But it also means that
; a read will block, until a write causes data to be placed into the
; queue. This kind of blocking behavior is what we want for pipeline
; designs: i.e. a pipeline always waits for data to show up, and is
; blocked while waiting.  However, this can lead to confusion if you
;; accidentally do too many reads, and then wonder why evertyhing seems
; to be hung. A write will unblock, but still can be confusing. You
; have been warned.

; First example: `pwd` -- where are we?
(cog-execute! (SetValue fsnode (Predicate "*-write-*")
	(Item "pwd")))

; The result of doing the pwd is obtained with a read.
(cog-execute! (ValueOf fsnode (Predicate "*-read-*")))

; -----------------------------------------------------------
; A scheme utility to run command-response pairs.
(define (run-fs-cmd CMD)
	(cog-execute! (SetValue fsnode (Predicate "*-write-*") CMD))
	(cog-execute! (ValueOf fsnode (Predicate "*-read-*"))))

(run-fs-cmd (Item "pwd"))
(run-fs-cmd (Item "ls"))
(run-fs-cmd (Item "special"))  ;; special files
(run-fs-cmd (Item "btime"))    ;; birth time
(run-fs-cmd (Item "mtime"))    ;; modification time
(run-fs-cmd (Item "filesize")) ;; filesize

(run-fs-cmd (List (Item "cd") (Item "file:///home")))
(run-fs-cmd (Item "ls"))

; --------------------------------------------------------
; The End! That's All, Folks!
