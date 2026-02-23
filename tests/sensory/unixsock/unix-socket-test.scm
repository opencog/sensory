#! /usr/bin/env guile
-s
!#
;
; unix-socket-test.scm -- Test UnixSocketNode basic operations
;
; Tests that UnixSocketNode can create a Unix domain socket, accept a
; client connection, and exchange text lines bidirectionally. The test
; acts as its own client, using Guile's built-in socket API to connect,
; so there is no dependency on external tools like socat.
;
(use-modules (opencog) (opencog sensory))
(use-modules (opencog test-runner))
(use-modules (ice-9 threads))

(opencog-test-runner)

(define tname "unix-socket-test")
(test-begin tname)

(define test-sock-path "/tmp/opencog-unix-socket-test.sock")

; Clean up any stale socket file from a previous run.
(catch #t
	(lambda () (delete-file test-sock-path))
	(lambda (key . args) #f))

; ----------------------------------------------------------
; Test 1: Open the socket and connect a client.
;
; UnixSocketNode::open() blocks in accept(), waiting for a client.
; So we open in a background thread, then connect from the main thread.

(define sock-node (UnixSocketNode (string-append "unix://" test-sock-path)))

; Open in a background Scheme thread.
(define open-exception #f)
(define open-thread
	(call-with-new-thread
		(lambda ()
			(catch #t
				(lambda ()
					(Trigger
						(SetValue sock-node
							(Predicate "*-open-*") (Type 'StringValue))))
				(lambda (key . args)
					(set! open-exception (cons key args)))))))

; Give the server thread time to create the socket and start listening.
(usleep 500000) ; 500ms

; Now connect as a client, using Guile's built-in socket support.
(define client-sock (socket AF_UNIX SOCK_STREAM 0))
(connect client-sock AF_UNIX test-sock-path)

; Wait for the open thread to finish (accept should now return).
(join-thread open-thread)

(test-assert "open-no-exception"
	(not open-exception))

; The socket file should exist on disk.
(test-assert "socket-file-exists"
	(file-exists? test-sock-path))

; ----------------------------------------------------------
; Test 2: Write from client, read from UnixSocketNode.
;
; Send a line of text from the client side. Then read it back
; using the *-read-* message on the UnixSocketNode.

(display "Hello from client\n" client-sock)
(force-output client-sock)

(define read-result
	(Trigger (ValueOf sock-node (Predicate "*-read-*"))))

(test-assert "read-from-client"
	(and (equal? 'StringValue (cog-type read-result))
	     (string-contains (cog-value-ref read-result 0) "Hello from client")))

; ----------------------------------------------------------
; Test 3: Write from UnixSocketNode, read from client.
;
; Write a line using the *-write-* message. Then read it from
; the client socket using Guile's read-line.

(Trigger
	(SetValue sock-node (Predicate "*-write-*")
		(StringValue "Hello from server\n")))

(define client-line (read-line client-sock))

(test-assert "write-to-client"
	(string-contains client-line "Hello from server"))

; ----------------------------------------------------------
; Test 4: Multiple round-trip exchanges.

(display "Line A\n" client-sock)
(force-output client-sock)
(display "Line B\n" client-sock)
(force-output client-sock)

(define read-a
	(Trigger (ValueOf sock-node (Predicate "*-read-*"))))
(define read-b
	(Trigger (ValueOf sock-node (Predicate "*-read-*"))))

(test-assert "multi-read-a"
	(and (equal? 'StringValue (cog-type read-a))
	     (string-contains (cog-value-ref read-a 0) "Line A")))

(test-assert "multi-read-b"
	(and (equal? 'StringValue (cog-type read-b))
	     (string-contains (cog-value-ref read-b 0) "Line B")))

; ----------------------------------------------------------
; Test 5: Close the socket.

; Close the client side first.
(close-port client-sock)

; Close the server side.
(Trigger
	(SetValue sock-node (Predicate "*-close-*") (Number 1)))

; After close, the socket file should be gone.
(test-assert "socket-file-removed"
	(not (file-exists? test-sock-path)))

; ----------------------------------------------------------
; Clean up, just in case.
(catch #t
	(lambda () (delete-file test-sock-path))
	(lambda (key . args) #f))

(test-end tname)

(opencog-test-end)
