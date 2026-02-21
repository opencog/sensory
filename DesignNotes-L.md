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

Option (3) above is the interesting one. This creates a pure atomese
description of what a vector DB should do. In essence, the Atomese can
be thought of as pseudo-code, except that its a bit more precise, since
Atomese is directly executable (with questionable performance, as well
as having other issues, like an unclear API specification).

Jigsaw API's
------------
At the center of the vector DB is the vector dot-product. One of the
Atomese demos implements that vector dot product.  It works because
All Atomese has C++ as an underlying default implementation. This
exposes two issues:

* There's no obvious way to "port" that Atomese to a GPU. Even is some
  automated Atomese-to-GPU compiler were built, there's no obvious
  location where to store that code (except as, duhh, a Value at some
  well-known key).
* There's no obvious way to specify the API to the lambda that
  implements the dot product. In the abstract, the notion of jigsaws
  and connectors provides the exactly-needed device for this. In
  practice, as seen in sensory-v0, the actual creation of hand-written
  API spec as jigsaws is untenable. Yes, the LG infrastructure can
  connect them up. But how is the lexis to be created?

The idea of using an LLM to provide text embeddings coupled to symbolic
systems, in a triple of (text, embedding, symbols) is that perhaps the
LLM can be used to smooth over the above issues. Maybe. I have an
inkling, here, the challenge is to work out the details.
