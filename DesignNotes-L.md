Memory, Assembly, Understanding
===============================
Below follows some ruminations on a simple design that could provide a
verbal (text) interface to complex strutural systems, as well as a
rudimentary form of memory. It is based on using an LLM, such as ollama,
to generate vector embeddings of text snippets; these vector embeddings
can then be associated with matching structural (symbolic) entities. It
might offer a simple way forward past some earlier design blockages
encountered in this project.

In this perspective, the LLM is treated as a natural language API into
non-verbal systems; the "thinking" happens non-verbally, while the text
LLM provides a way of manipulating, controlling and working with th
non-verbal elements. The idea is to build on the concept of RAG
(retreival-augumented generation) or semantic search, by building
triples of (text, vector-embedding-of-text, symbolic-structure).

A "memory" architecture
-----------------------
The core archtiecture builds on the conventional notion of a
conversational context, or of a memory subsystem.  Here, some large
collection of text paragraphs (e.g. my diary, or perhaps these design
notes) are split up into paragraph (or half-page) sized chunks. These
are run through ollama to get embedding vectors (qwen3:8b offers 4K
float vectors; earlier models offer shorter vectors). The pairing of
(text, vect) is stored in a vector database; input queries use cosine
similarity to find the most-similar vectors, and thus the associated
text.

A list of the obvious issues:
* The vector embedding depends entirely on the model; embeddings are not
  portable.
* Besides Ollama, there's also HugginFace. Commercial embeddings are
  available from OpenAI and Google, but these have the drawback of being
  commercial.
* I need a vector DB. There are two possibilities: (1) create an Atomese
  wrapper around Faise, the open source facebook vector DB, (2) create a
  pure-atomese implementation of a vector DB, or (3) do both.

Option (3) above is the interesting one
