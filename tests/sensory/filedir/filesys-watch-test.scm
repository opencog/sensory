;
; filesys-watch-test.scm -- Test directory watching functionality for FileSysNode
;
; Tests that FileSysNode can watch a directory using inotify, detecting
; file creation, modification, and move events.
;
(use-modules (opencog) (opencog exec) (opencog sensory))
(use-modules (opencog test-runner))

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
; Test 3: File move detection (MOVED_TO event)

; Create a file outside the watched directory and move it in
; This generates only an IN_MOVED_TO event (not IN_MOVED_FROM)
(system "sleep 1 && touch /tmp/test-move-source.txt &")
(usleep 500000) ; Wait for file creation
(system (string-append "sleep 1 && mv /tmp/test-move-source.txt " test-dir "/test2.txt &"))

; Read the MOVED_TO event
(define event3 (cog-execute! (ValueOf fsnode (Predicate "*-read-*"))))
(test-assert "file-move-detected"
	(and (equal? 'StringValue (cog-type event3))
	     (string-contains (cog-value-ref event3 0) ".txt")))

; ----------------------------------------------------------
; Test 4: Multiple files created sequentially

; Create files one at a time to ensure reliable event delivery
(system (string-append "sleep 1 && touch " test-dir "/file1.txt &"))
(define multi-event1 (cog-execute! (ValueOf fsnode (Predicate "*-read-*"))))

(system (string-append "sleep 1 && touch " test-dir "/file2.txt &"))
(define multi-event2 (cog-execute! (ValueOf fsnode (Predicate "*-read-*"))))

(system (string-append "sleep 1 && touch " test-dir "/file3.txt &"))
(define multi-event3 (cog-execute! (ValueOf fsnode (Predicate "*-read-*"))))

; All three should be StringValues containing filenames
(test-assert "multiple-files-detected"
	(and (equal? 'StringValue (cog-type multi-event1))
	     (equal? 'StringValue (cog-type multi-event2))
	     (equal? 'StringValue (cog-type multi-event3))
	     (string-contains (cog-value-ref multi-event1 0) ".txt")
	     (string-contains (cog-value-ref multi-event2 0) ".txt")
	     (string-contains (cog-value-ref multi-event3 0) ".txt")))

; ----------------------------------------------------------
; Test 5: StreamValueOf returns samples wrapped in LinkValue

; Create first test file for streaming
(system (string-append "sleep 1 && touch " test-dir "/stream1.txt &"))

; Read using StreamValueOf - should return a LinkValue containing the sample
(define stream-event1 (cog-execute! (StreamValueOf fsnode (Predicate "*-stream-*"))))
(test-assert "stream-value-returns-linkvalue"
	(equal? 'LinkValue (cog-type stream-event1)))

; The LinkValue should contain one StringValue element
(test-assert "stream-value-contains-string"
	(and (= 1 (cog-arity stream-event1))
	     (equal? 'StringValue (cog-type (cog-value-ref stream-event1 0)))
	     (string-contains (cog-value-ref (cog-value-ref stream-event1 0) 0) ".txt")))

; Create second test file
(system (string-append "sleep 1 && touch " test-dir "/stream2.txt &"))

; Read another event using StreamValueOf
(define stream-event2 (cog-execute! (StreamValueOf fsnode (Predicate "*-stream-*"))))
(test-assert "stream-value-second-sample"
	(and (equal? 'LinkValue (cog-type stream-event2))
	     (= 1 (cog-arity stream-event2))
	     (equal? 'StringValue (cog-type (cog-value-ref stream-event2 0)))
	     (string-contains (cog-value-ref (cog-value-ref stream-event2 0) 0) ".txt")))

; ----------------------------------------------------------
; Test 6: Compare StreamValueOf vs ValueOf behavior

; Create a file to compare the two methods
(system (string-append "sleep 1 && touch " test-dir "/compare.txt &"))

; Read with ValueOf - returns unwrapped sample
(define valueof-result (cog-execute! (ValueOf fsnode (Predicate "*-read-*"))))
(test-assert "valueof-returns-stringvalue"
	(equal? 'StringValue (cog-type valueof-result)))

; Create another file for StreamValueOf
(system (string-append "sleep 1 && touch " test-dir "/stream-compare.txt &"))

; Read with StreamValueOf - returns wrapped sample
(define streamvalueof-result (cog-execute! (StreamValueOf fsnode (Predicate "*-stream-*"))))
(test-assert "streamvalueof-wraps-in-linkvalue"
	(and (equal? 'LinkValue (cog-type streamvalueof-result))
	     (= 1 (cog-arity streamvalueof-result))
	     (equal? 'StringValue (cog-type (cog-value-ref streamvalueof-result 0)))))

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
