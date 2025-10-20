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
; Open the filesystem node. The URL must be of the form
;    `file:///somewhere/anything`
; The file path does not have to actually exist, even at open time;
; however, subsequent commands will throw an exception if applied to
; non-existent directories or files. Thus, the first command after an
; open of a non-existent path should be a `cd` to a valid path.
;
; XXX TODO: maybe change design to return VoidValue, instead of
; throwing?

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
;; accidentally do too many reads, and then wonder why everything seems
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
; Directories can also be watched for file additions, removals and
; changes. This interfaces works a bit differently than the above:
; the changes are streamed, as they occur, rather than returned in
; one blob. Here, the stream functions as an "event queue", holding
; changes that have been made to files in the directory. The events
; can be dequeued, one by one, or they can be streamed. (Recall that
; "streaming" means that these events can be processed through some
; pipeline written in Atomese.) Stream continues indefinitely, until
; the node is closed (sent the `*-close-*` message.)
;
; To keep things distinct, the demo will use a different demo directory.

(mkdir "/tmp/demo-watch")
(define watchnode (FileSysNode "file:///tmp/demo-watch"))
(cog-set-value! watchnode (Predicate "*-open-*") (Type 'StringValue))
(cog-set-value! watchnode (Predicate "*-write-*") (StringValue "watch"))

; The stream can be accessed two different ways: by reading events,
; one at a time, from the event queue, or by streaming. The
; one-at-a-time API is provided by the `*-read-*` message, which
; dequeues one event and returns it. (It will block, if the queue
; is empty.) The `*-stream-*` message returns a handle to the stream.
; The stream can be sampled, one at a time, with the `StreamValueOf`
; link.

; Run the below. In another terminal, create, destroy or edit
; some file in the `/tmp/demo-watch` directory.  Note that this
; will hang, until there is some change. Repeated calls will return
; those changes, one at a time.
(cog-execute! (ValueOf watchnode (Predicate "*-read-*")))

; The `StreamValueOf` link behaves similarly, taking one sample from
; the stream each time that it is executed. CAUTION: Do NOT attempt
; `ValueOf` here. The `ValueOf` will function correctly, and return
; the stream handle. However, the guile scheme shell, when it attempts
; to print this stream, will hang: the print will not return until
; the stream closes. With some cleverness, it can be closed from a
; different thread, if you've arranged for that thread in advance.
; If not, you will hard-hang.
(cog-execute! (StreamValueOf watchnode (Predicate "*-stream-*")))

; --------------------------------------------------------
; The End! That's All, Folks!
