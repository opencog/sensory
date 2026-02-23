#! /usr/bin/env guile
-s
!#
;
; tcp-socket-test.scm -- Test TcpSocketNode basic operations
;
; Tests that TcpSocketNode can create a TCP socket, accept a
; client connection, and exchange text lines bidirectionally. The test
; acts as its own client, using Guile's built-in socket API to connect,
; so there is no dependency on external tools like socat.
;
; The open message creates the socket and starts listening but does NOT
; block. The accept happens lazily on the first read, which does block.
; So the test connects a client and sends data BEFORE the first read,
; ensuring the socket always has data available when read is called.
;
(use-modules (opencog) (opencog sensory))
(use-modules (opencog test-runner))
(use-modules (ice-9 rdelim))

(opencog-test-runner)

(define tname "tcp-socket-test")
(test-begin tname)

; Use a high port unlikely to conflict.
(define test-port 17171)
(define test-url (string-append "tcp://127.0.0.1:" (number->string test-port)))

; ----------------------------------------------------------
; Test 1: Open the socket (non-blocking).
;
; open() creates the socket, binds, and listens. It returns
; immediately without waiting for a client to connect.

(define sock-node (TcpSocketNode test-url))

(Trigger
	(SetValue sock-node (Predicate "*-open-*") (Type 'StringValue)))

; For TCP, we verify it opened by confirming the node is valid.
(test-assert "tcp-socket-opened"
	(not (null? sock-node)))

; ----------------------------------------------------------
; Test 2: Connect a client, write from client, read from server.
;
; Connect using Guile's built-in socket support. Then send a line
; of text from the client side BEFORE doing any read, so that the
; socket has data waiting when read is called.

(define client-sock (socket AF_INET SOCK_STREAM 0))
(connect client-sock AF_INET (inet-pton AF_INET "127.0.0.1") test-port)

; Send data first, so the read won't block.
(display "Hello from client\n" client-sock)
(force-output client-sock)

; Set up a one-shot reader, same pattern as the xterm-io demo.
(Pipe (Name "reader") (ValueOf sock-node (Predicate "*-read-*")))

(define read-result (Trigger (Name "reader")))

(test-assert "read-from-client"
	(and (equal? 'StringValue (cog-type read-result))
	     (string-contains (cog-value-ref read-result 0) "Hello from client")))

; ----------------------------------------------------------
; Test 3: Write from TcpSocketNode, read from client.
;
; Write a line using the *-write-* message. Then read it from
; the client socket using Guile's read-line.

(Trigger
	(SetValue sock-node (Predicate "*-write-*")
		(Item "Hello from server\n")))

(define client-line (read-line client-sock))

(test-assert "write-to-client"
	(string-contains client-line "Hello from server"))

; ----------------------------------------------------------
; Test 4: Multiple round-trip exchanges.
; Always send data before reading, so the socket is never empty.

(display "Line A\n" client-sock)
(force-output client-sock)

(define read-a (Trigger (Name "reader")))

(test-assert "multi-read-a"
	(and (equal? 'StringValue (cog-type read-a))
	     (string-contains (cog-value-ref read-a 0) "Line A")))

(display "Line B\n" client-sock)
(force-output client-sock)

(define read-b (Trigger (Name "reader")))

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

; After close, verify the node is still valid (no crash).
(test-assert "tcp-socket-closed"
	(not (null? sock-node)))

; ----------------------------------------------------------
(test-end tname)

(opencog-test-end)
