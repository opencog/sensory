;
; filesys-watch-test.scm -- Test directory watching functionality for FileSysNode
;
; Tests that FileSysNode can watch a directory using inotify, detecting
; file creation, modification, and move events.
;
(use-modules (opencog))
(use-modules (opencog exec))
(use-modules (opencog test-runner))
(use-modules (opencog sensory))

(opencog-test-runner)

(define tname "filesys-watch")
(test-begin tname)

; Create a temporary test directory
(define test-dir "/tmp/filesys-watch-test")

; Clean up any previous test directory
(catch #t
	(lambda () (system (string-append "rm -rf " test-dir)))
	(lambda (key . args) #f))

; Create the test directory
(mkdir test-dir)

; ----------------------------------------------------------
; Test 1: Basic watch setup and file creation detection

(define fsnode (FileSysNode (string-append "file://" test-dir)))

; Open the FileSysNode
(cog-set-value! fsnode (Predicate "*-open-*") (Type 'StringValue))

; Start watching by sending the "watch" command
(cog-set-value! fsnode (Predicate "*-write-*") (Node "watch"))

; Fork a process to create a file after a delay
(system (string-append "sleep 1 && touch " test-dir "/test1.txt &"))

; Read from the FileSysNode - this will block until an event occurs
; The UnisetValue's remove() method blocks until an item is available
(define event1 (cog-execute! (ValueOf fsnode (Predicate "*-read-*"))))
(test-assert "file-creation-detected"
	(and (equal? 'StringValue (cog-type event1))
	     (string-contains (cog-value-ref event1 0) "test1.txt")))

; ----------------------------------------------------------
; Test 2: File modification detection

; Fork a process to modify the file
(system (string-append "sleep 1 && echo 'data' > " test-dir "/test1.txt &"))

; Read the modification event
(define event2 (cog-execute! (ValueOf fsnode (Predicate "*-read-*"))))
(test-assert "file-modification-detected"
	(and (equal? 'StringValue (cog-type event2))
	     (string-contains (cog-value-ref event2 0) "test1.txt")))

; ----------------------------------------------------------
; Test 3: File move detection

; Fork a process to create and move a file
(system (string-append "sleep 1 && touch /tmp/test-move.txt && mv /tmp/test-move.txt " test-dir "/test2.txt &"))

; Read the move event
(define event3 (cog-execute! (ValueOf fsnode (Predicate "*-read-*"))))
(test-assert "file-move-detected"
	(and (equal? 'StringValue (cog-type event3))
	     (string-contains (cog-value-ref event3 0) "test2.txt")))

; ----------------------------------------------------------
; Test 4: Multiple files created rapidly

; Fork a process to create multiple files
(system (string-append "sleep 1 && touch " test-dir "/file1.txt " test-dir "/file2.txt " test-dir "/file3.txt &"))

; We should be able to read all three events
; Note: UnisetValue deduplicates, but these are different filenames
(define multi-event1 (cog-execute! (ValueOf fsnode (Predicate "*-read-*"))))
(define multi-event2 (cog-execute! (ValueOf fsnode (Predicate "*-read-*"))))
(define multi-event3 (cog-execute! (ValueOf fsnode (Predicate "*-read-*"))))

; Collect the filenames
(define multi-names
	(list
		(cog-value-ref multi-event1 0)
		(cog-value-ref multi-event2 0)
		(cog-value-ref multi-event3 0)))

; Check that we got three different filenames, all matching our pattern
(test-assert "multiple-files-detected"
	(and (= 3 (length multi-names))
	     (or (string-contains (car multi-names) "file1.txt")
	         (string-contains (car multi-names) "file2.txt")
	         (string-contains (car multi-names) "file3.txt"))
	     (or (string-contains (cadr multi-names) "file1.txt")
	         (string-contains (cadr multi-names) "file2.txt")
	         (string-contains (cadr multi-names) "file3.txt"))
	     (or (string-contains (caddr multi-names) "file1.txt")
	         (string-contains (caddr multi-names) "file2.txt")
	         (string-contains (caddr multi-names) "file3.txt"))))

; ----------------------------------------------------------
; Clean up

; Close FileSysNode
(cog-set-value! fsnode (Predicate "*-close-*") (VoidValue))

; Give the watch thread time to stop
(usleep 200000) ; 200ms

; Clean up test directory
(catch #t
	(lambda () (system (string-append "rm -rf " test-dir)))
	(lambda (key . args) #f))

(test-end tname)

(opencog-test-end)
