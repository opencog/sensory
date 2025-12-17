#! /usr/bin/env guile
-s
!#
;
; tail-follow-test.scm -- Test tail/follow functionality for TextFileNode
;
; Tests that TextFileNode can tail a file using inotify, waiting for
; new lines to be appended and returning them as a stream.
;
(use-modules (opencog))
(use-modules (opencog exec))
(use-modules (opencog test-runner))
(use-modules (opencog sensory))

(opencog-test-runner)

(define tname "tail-follow")
(test-begin tname)

; Create a temporary test file
(define test-file "/tmp/tail-follow-test.txt")

; Clean up any previous test file
(catch #t
	(lambda () (delete-file test-file))
	(lambda (key . args) #f))

; Create the file with initial content
(with-output-to-file test-file
	(lambda ()
		(display "Line 1\n")
		(display "Line 2\n")))

; ----------------------------------------------------------
; Test 1: Normal mode (non-tail) reads all lines and stops at EOF

(define file-node-normal (TextFile (string-append "file://" test-file)))

; Open in normal mode (no follow) - use Item type to get Nodes back
(cog-execute! (SetValue file-node-normal (Predicate "*-open-*") (Type 'Item)))

; Read first line using *-read-* message
(define line1 (cog-execute! (ValueOf file-node-normal (Predicate "*-read-*"))))
(test-assert "normal-mode-line1"
	(string-contains (cog-name line1) "Line 1"))

; Read second line
(define line2 (cog-execute! (ValueOf file-node-normal (Predicate "*-read-*"))))
(test-assert "normal-mode-line2"
	(string-contains (cog-name line2) "Line 2"))

; Read EOF - should return VoidValue
(define eof-marker (cog-execute! (ValueOf file-node-normal (Predicate "*-read-*"))))
(test-assert "normal-mode-eof"
	(equal? 'VoidValue (cog-type eof-marker)))

; ----------------------------------------------------------
; Test 2: Tail mode waits for new content

; Create new file node for tail mode
(define file-node-tail (TextFile (string-append "file://" test-file)))

; Reset test file
(with-output-to-file test-file
	(lambda ()
		(display "Start\n")))

; Enable tail mode BEFORE opening
(cog-set-value! file-node-tail (Predicate "*-follow-*") (BoolValue #t))

; Open in tail mode - use Item type to get Nodes back
(cog-execute! (SetValue file-node-tail (Predicate "*-open-*") (Type 'Item)))

; Read the initial line using *-read-* message
(define start-line (cog-execute! (ValueOf file-node-tail (Predicate "*-read-*"))))
(test-assert "tail-mode-initial"
	(string-contains (cog-name start-line) "Start"))

; Now we're at EOF. In tail mode, the next read will block waiting for more data.
; To test this, we need to append to the file from another thread.
; For simplicity, we'll use a shell command to append after a short delay.

; Fork a process to append after a delay
(system (string-append "sleep 1 && echo 'Appended line' >> " test-file " &"))

; This read should block until the append happens, then return the new line
; Note: This will wait up to ~1 second for the append
(define appended-line (cog-execute! (ValueOf file-node-tail (Predicate "*-read-*"))))
(test-assert "tail-mode-appended"
	(string-contains (cog-name appended-line) "Appended"))

; Test that we can disable tail mode
(cog-set-value! file-node-tail (Predicate "*-follow-*") (BoolValue #f))

; After disabling tail mode, reading at EOF should return VoidValue immediately
; (not block waiting for more data)
(define eof-after-disable (cog-execute! (ValueOf file-node-tail (Predicate "*-read-*"))))
(test-assert "tail-mode-disabled-eof"
	(equal? 'VoidValue (cog-type eof-after-disable)))

; ----------------------------------------------------------
; Clean up

; Close files - using Number 1 as a simple atom to trigger close
(cog-execute! (SetValue file-node-normal (Predicate "*-close-*") (Number 1)))
(cog-execute! (SetValue file-node-tail (Predicate "*-close-*") (Number 1)))

(catch #t
	(lambda () (delete-file test-file))
	(lambda (key . args) #f))

(test-end tname)

(opencog-test-end)
