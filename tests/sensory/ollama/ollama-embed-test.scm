#! /usr/bin/env guile
-s
!#
;
; ollama-embed-test.scm -- Test OllamaNode embedding API
;
; Tests that OllamaNode can compute embeddings via the *-embedding-*
; message. Requires the nomic-embed-text model to be pulled:
;
;   ollama serve
;   ollama pull nomic-embed-text
;
; If the Ollama server is not already running, this test will start it
; and stop it when done.
;
(use-modules (opencog) (opencog sensory))
(use-modules (opencog test-runner))
(use-modules (srfi srfi-1))
(use-modules (ice-9 popen))
(use-modules (ice-9 rdelim))

(opencog-test-runner)

(define tname "ollama-embed-test")
(test-begin tname)

; The embedding model to use. nomic-embed-text produces 768-dim vectors.
(define embed-model "nomic-embed-text")
(define embed-url
	(string-append "ollama://localhost:11434/" embed-model))

; ----------------------------------------------------------
; Start the Ollama server if it is not already running.
; We probe port 11434 by attempting a TCP connection; if it fails,
; we launch "ollama serve" in the background and wait for it to
; become ready.

(define started-ollama #f)

(define (ollama-server-running?)
	"Return #t if something is listening on localhost:11434."
	(catch #t
		(lambda ()
			(let ((s (socket AF_INET SOCK_STREAM 0)))
				(connect s AF_INET (inet-pton AF_INET "127.0.0.1") 11434)
				(close-port s)
				#t))
		(lambda (key . args) #f)))

(define (wait-for-ollama max-tries)
	"Block until the Ollama server is accepting connections."
	(let loop ((n 0))
		(when (< n max-tries)
			(if (not (ollama-server-running?))
				(begin (sleep 1) (loop (+ n 1)))))))

(when (not (ollama-server-running?))
	(format #t "Ollama server not running; starting 'ollama serve' ...\n")
	; Launch in the background; redirect output so it doesn't clutter
	; the test output.
	(system "ollama serve > /dev/null 2>&1 &")
	(set! started-ollama #t)
	(wait-for-ollama 30)
	(when (not (ollama-server-running?))
		(format #t "ERROR: Ollama server failed to start\n")
		(test-end tname)
		(opencog-test-end)
		(exit 1)))

; ----------------------------------------------------------
; Test 1: Open a connection with an embedding model.

(define emb-node (OllamaNode "test-embedder"))

(Trigger
	(SetValue emb-node (Predicate "*-open-*")
		(Item embed-url)))

(test-assert "embed-node-connected"
	(equal? #t
		(cog-value-ref
			(Trigger (ValueOf emb-node (Predicate "*-connected?-*")))
			0)))

; ----------------------------------------------------------
; Test 2: Compute an embedding and verify it returns a Float32Value.

(Trigger
	(SetValue emb-node (Predicate "*-embedding-*")
		(Item "The quick brown fox jumps over the lazy dog.")))

(define emb1 (cog-value emb-node (Predicate "*-embedding-*")))

(test-assert "embedding-is-float32value"
	(equal? 'Float32Value (cog-type emb1)))

; ----------------------------------------------------------
; Test 3: Verify the embedding vector has the expected dimensionality.
; nomic-embed-text produces 768-dimensional vectors.

(define emb1-list (cog-value->list emb1))

(test-assert "embedding-dimension-768"
	(= 768 (length emb1-list)))

; ----------------------------------------------------------
; Test 4: Verify the embedding contains non-trivial floats
; (not all zeros).

(test-assert "embedding-not-all-zeros"
	(not (every (lambda (x) (= 0.0 x)) emb1-list)))

; ----------------------------------------------------------
; Test 5: A second embedding for different text should produce
; a different vector.

(Trigger
	(SetValue emb-node (Predicate "*-embedding-*")
		(Item "Lambda calculus is a formal system in mathematical logic.")))

(define emb2 (cog-value emb-node (Predicate "*-embedding-*")))
(define emb2-list (cog-value->list emb2))

(test-assert "second-embedding-is-float32value"
	(equal? 'Float32Value (cog-type emb2)))

(test-assert "second-embedding-dimension-768"
	(= 768 (length emb2-list)))

; Check that the two embeddings differ (at least one element differs).
(test-assert "different-texts-different-embeddings"
	(not (equal? emb1-list emb2-list)))

; ----------------------------------------------------------
; Test 6: SetValueOn chaining pattern -- compute and read in one shot.

(define emb3 (Trigger
	(ValueOf
		(SetValueOn emb-node (Predicate "*-embedding-*")
			(Item "A third distinct sentence for testing."))
		(Predicate "*-embedding-*"))))

(test-assert "setvalueon-returns-float32value"
	(equal? 'Float32Value (cog-type emb3)))

(test-assert "setvalueon-dimension-768"
	(= 768 (length (cog-value->list emb3))))

; ----------------------------------------------------------
; Test 7: Close the connection.

(Trigger
	(SetValue emb-node (Predicate "*-close-*") (Number 1)))

(test-assert "embed-node-closed"
	(equal? #f
		(cog-value-ref
			(Trigger (ValueOf emb-node (Predicate "*-connected?-*")))
			0)))

; ----------------------------------------------------------
; Stop the Ollama server if we started it.
(when started-ollama
	(format #t "Stopping Ollama server ...\n")
	(system "ollama stop > /dev/null 2>&1")
	; Give it a moment to shut down, then kill the serve process
	; in case "ollama stop" only unloads models.
	(sleep 1)
	(system "pkill -f 'ollama serve' > /dev/null 2>&1"))

; ----------------------------------------------------------
(test-end tname)

(opencog-test-end)
