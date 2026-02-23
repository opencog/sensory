;
; ollama-embed.scm -- Ollama embedding API demo
;
; Demo of the embedding interface. This computes a vector embedding
; for a text string, using an Ollama embedding model such as
; nomic-embed-text or mxbai-embed-large.
;
; Prerequisites:
;   ollama serve
;   ollama pull nomic-embed-text
;
(use-modules (opencog) (opencog sensory))
(use-modules (srfi srfi-1))

; --------------------------------------------------------
; Open a connection to Ollama with an embedding model.
; Embedding models are different from chat models -- they produce
; float vectors instead of text.
(Trigger
	(SetValue (OllamaNode "my-embedder") (Predicate "*-open-*")
		(Item "ollama://localhost:11434/nomic-embed-text")))

; --------------------------------------------------------
; Compute an embedding. This is synchronous -- it blocks until the
; result is ready, then stores a Float32Value at the *-embedding-* key.
(Trigger
	(SetValue (OllamaNode "my-embedder") (Predicate "*-embedding-*")
		(Item "The quick brown fox jumps over the lazy dog.")))

; Read the embedding vector back.
(define emb (cog-value (OllamaNode "my-embedder") (Predicate "*-embedding-*")))
(format #t "Embedding type: ~A\n" (cog-type emb))
(format #t "Embedding size: ~A\n" (length (cog-value->list emb)))
(format #t "First 5 floats: ~A\n" (take (cog-value->list emb) 5))

; --------------------------------------------------------
; The same thing can be done in one expression using SetValueOn.
; SetValueOn calls setValue (which computes the embedding), then
; returns the node itself, so ValueOf can read the result.
(define emb2 (Trigger
	(ValueOf
		(SetValueOn (OllamaNode "my-embedder") (Predicate "*-embedding-*")
			(Item "Lambda calculus is a formal system in mathematical logic."))
		(Predicate "*-embedding-*"))))

(format #t "\nSecond embedding size: ~A\n"
	(length (cog-value->list emb2)))

; --------------------------------------------------------
; Close the connection.
(Trigger
	(SetValue (OllamaNode "my-embedder") (Predicate "*-close-*")))

; --------------------------------------------------------
; The End! That's All, Folks!
