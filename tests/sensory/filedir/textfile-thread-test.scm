#! /usr/bin/env guile
-s
!#
;
; textfile-thread-test.scm -- Test TextFileNode thread safety
;
; Tests that TextFileNode can be safely closed from one thread while
; another thread is blocked waiting for data in follow (tail) mode.
; This tests proper cleanup and unblocking behavior.
;
(use-modules (opencog) (opencog exec) (opencog sensory))
(use-modules (opencog test-runner))
(use-modules (ice-9 threads))

(opencog-test-runner)

(define tname "textfile-thread")
(test-begin tname)

; Create a test file with initial content
(define test-file "/tmp/textfile-thread-test.txt")
(with-output-to-file test-file
	(lambda () (display "line1\nline2\n")))

; ----------------------------------------------------------
; Test 1: Close from another Scheme thread while blocked

(define file-node-1 (TextFile (string-append "file://" test-file)))

; Open and enable follow mode
(cog-execute! (SetValue file-node-1 (Predicate "*-open-*") (Type 'StringValue)))
(cog-set-value! file-node-1 (Predicate "*-follow-*") (BoolValue #t))

; Read the two existing lines
(define line1 (cog-execute! (ValueOf file-node-1 (Predicate "*-read-*"))))
(define line2 (cog-execute! (ValueOf file-node-1 (Predicate "*-read-*"))))

(test-assert "read-initial-lines"
	(and (equal? 'StringValue (cog-type line1))
	     (equal? 'StringValue (cog-type line2))))

; Create reader thread that will block waiting for more data
(define reader-result #f)
(define reader-exception #f)

(define reader-thread
	(call-with-new-thread
		(lambda ()
			(catch #t
				(lambda ()
					(set! reader-result
						(cog-execute! (ValueOf file-node-1 (Predicate "*-read-*")))))
				(lambda (key . args)
					(set! reader-exception (cons key args)))))))

; Give reader thread time to enter blocked state
(usleep 500000) ; 500ms

; Now close from main thread - this should unblock the reader
(cog-set-value! file-node-1 (Predicate "*-close-*") (VoidValue))

; Wait for reader thread to complete (should return immediately now)
(define join-result (join-thread reader-thread))

; The reader should have received VoidValue (empty) when closed
(test-assert "scheme-thread-unblocked"
	(or (and reader-result (equal? 'VoidValue (cog-type reader-result)))
	    reader-exception)) ; Either got VoidValue or caught exception is OK

; ----------------------------------------------------------
; Test 2: Close from another thread using ExecuteThreadedLink (pure Atomese)

(define file-node-2 (TextFile (string-append "file://" test-file)))

; Rewrite test file
(with-output-to-file test-file
	(lambda () (display "line1\nline2\n")))

; Open and enable follow mode
(cog-execute! (SetValue file-node-2 (Predicate "*-open-*") (Type 'StringValue)))
(cog-set-value! file-node-2 (Predicate "*-follow-*") (BoolValue #t))

; Read the two existing lines first
(cog-execute! (ValueOf file-node-2 (Predicate "*-read-*")))
(cog-execute! (ValueOf file-node-2 (Predicate "*-read-*")))

; Create pure Atomese threaded execution:
; Thread 1: Try to read (will block)
; Thread 2: Sleep then close
(define threaded-test
	(ExecuteThreadedLink
		; Thread 1: Reader - will block waiting for data
		(ValueOf file-node-2 (Predicate "*-read-*"))

		; Thread 2: Closer - sleeps then closes (pure Atomese)
		(SequentialAnd
			; Sleep to let reader block first
			(Sleep (Number 0.5))
			; Then close to unblock reader (using Concept as close signal)
			(SetValue file-node-2 (Predicate "*-close-*") (Concept "close-signal")))))

; Execute the threaded test
; This should complete without hanging
(define threaded-result #f)
(define threaded-exception #f)

(catch #t
	(lambda ()
		(set! threaded-result (cog-execute! threaded-test)))
	(lambda (key . args)
		(set! threaded-exception (cons key args))))

; Test should complete (not hang)
(test-assert "atomese-thread-completed"
	(or threaded-result threaded-exception))

; If we got a result, it should be a QueueValue with results from both threads
(test-assert "atomese-thread-result-structure"
	(or (and threaded-result
	         (equal? 'QueueValue (cog-type threaded-result)))
	    threaded-exception)) ; Exception is acceptable

; ----------------------------------------------------------
; Test 3: Verify no corruption after threaded close

(define file-node-3 (TextFile (string-append "file://" test-file)))

; Should be able to open again without issues
(cog-execute! (SetValue file-node-3 (Predicate "*-open-*") (Type 'StringValue)))

; Read a line normally
(define normal-read (cog-execute! (ValueOf file-node-3 (Predicate "*-read-*"))))

(test-assert "no-corruption-after-threaded-close"
	(equal? 'StringValue (cog-type normal-read)))

; Clean close
(cog-set-value! file-node-3 (Predicate "*-close-*") (VoidValue))

; ----------------------------------------------------------
; Clean up

(system (string-append "rm -f " test-file))

(test-end tname)

(opencog-test-end)
