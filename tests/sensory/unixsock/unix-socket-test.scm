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
; The open message creates the socket and starts listening but does NOT
; block. The accept happens lazily on the first read, which does block.
; So the test connects a client between the open and the first read.
;
(use-modules (opencog) (opencog sensory))
(use-modules (opencog test-runner))
(use-modules (ice-9 rdelim))

(opencog-test-runner)

(define tname "unix-socket-test")
(test-begin tname)

(define test-sock-path "/tmp/opencog-unix-socket-test.sock")

; Clean up any stale socket file from a previous run.
(catch #t
	(lambda () (delete-file test-sock-path))
	(lambda (key . args) #f))

; ----------------------------------------------------------
; Test 1: Open the socket (non-blocking).
;
; open() creates the socket, binds, and listens. It returns
; immediately without waiting for a client to connect.

(define sock-node (UnixSocketNode (string-append "unix://" test-sock-path)))

(Trigger
	(SetValue sock-node (Predicate "*-open-*") (Type 'StringValue)))

; The socket file should exist on disk immediately after open.
(test-assert "socket-file-exists"
	(file-exists? test-sock-path))

; ----------------------------------------------------------
; Test 2: Connect a client, write from client, read from server.
;
; Connect using Guile's built-in socket support. Then send a line
; of text from the client side and read it back via *-read-*.
; The first read triggers accept() internally.

(define client-sock (socket AF_UNIX SOCK_STREAM 0))
(connect client-sock AF_UNIX test-sock-path)

; Send data before reading. The data will be buffered in the kernel
; socket until the server side reads it.
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
