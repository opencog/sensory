;
; ollama-api.scm -- Ollama LLM API demo
;
; Demo of the basic Ollama interface. The API offers a simple way to
; connect to an Ollama server and exchange prompts/responses.
;
(use-modules (opencog) (opencog sensory))

; --------------------------------------------------------
; Open a connection to an Ollama node.
; The URL format is ollama://host:port/model
;
; Open a connection to a locally-running Ollama with the qwen3:8b model.
(Trigger
	(SetValue (OllamaNode "my-llm") (Predicate "*-open-*")
		(Item "ollama://localhost:11434/qwen3:8b")))

; --------------------------------------------------------
; Send a prompt. This uses the /api/generate endpoint.
; The write is asynchronous -- it queues the request and a
; background thread sends it to Ollama.
(Trigger
	(SetValue (Ollama "my-llm") (Predicate "*-write-*")
		(Item "What is the capital of France?")))

; Read the response. This blocks until the LLM finishes generating.
; Caution: this will block the current thread (the main thread) if
; called a second time, without a preceeding *-write-*.
(Trigger
	(ValueOf (Ollama "my-llm") (Predicate "*-read-*")))

; Send another prompt.
(Trigger
	(SetValue (Ollama "my-llm") (Predicate "*-write-*")
		(Item "Explain lambda calculus in three sentences.")))

; Read that response.
(Trigger
	(ValueOf (Ollama "my-llm") (Predicate "*-read-*")))

; --------------------------------------------------------
; Check if connected.
(Trigger
	(ValueOf (Ollama "my-llm") (Predicate "*-connected?-*")))

; --------------------------------------------------------
; A bare model name also works, defaulting to localhost:11434:
;
; (SetValue (Ollama "my-llm") (Predicate "*-open-*")
;	 (Concept "qwen3:8b"))

; --------------------------------------------------------
; Close the connection.
(Trigger
	(SetValue (Ollama "my-llm") (Predicate "*-close-*")))

; --------------------------------------------------------
; The End! That's All, Folks!
