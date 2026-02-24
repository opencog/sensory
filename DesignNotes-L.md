Memory, Assembly, Understanding
===============================
Below follows some ruminations on a simple design that could provide a
verbal (text) interface to complex strutural systems, as well as a
rudimentary form of memory. It is based on using an LLM, such as ollama,
to generate vector embeddings of text snippets; these vector embeddings
can then be associated with matching structural (symbolic) entities. It
might offer a simple way forward past some earlier design blockages
encountered in this project.

The perspective is that of "semantic routing": the LLM is treated as a
natural language API into non-verbal systems; the "thinking" happens
non-verbally, while the text LLM provides a way of manipulating,
controlling and working with the non-verbal elements. The idea is to
build on the concept of RAG (retreival-augumented generation), semantic
search or semantic routing, by building triples of
```
   (text, vector-embedding-of-text, symbolic-structure).
```
This is a pre-MCP conception of the interface, in part because Ollama
does not have MCP support, and in part because MCP does not do what I
want it to do; and so I'm exploring some more basic technologies.


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
  wrapper around Faiss, the open source facebook vector DB, (2) create a
  pure-atomese implementation of a vector DB, or (3) do both.

Option (3) above is the interesting one. This creates a pure atomese
description of what a vector DB should do. In essence, the Atomese can
be thought of as pseudo-code, except that its a bit more precise, since
Atomese is directly executable (with questionable performance, as well
as having other issues, like an unclear API specification).

FWIW, the pure-Atomese version needs to also implement the search algo.
Claude mentions HNSW, IVF and more:
* ColBERT -- vectorize tokes, sum max tokens
* (Others that seem not relevant here)

Off-the-shelf
-------------
Some existing systems:
* LangChain and LlamaIndex provide MCP-like API's
* Gorilla LLM -- specifically trained for tool calling.


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

Storyboard
----------
How might this work? At the simplest level, it looks like "skills":
there's an English-language paragraph that says "To find all files with
filename X, run the following query." and then there is the actual
Atomese, stored as the third part of that triple.

The first stumbling block is how to plug in the value for X into the
Atomese. The standard solution is "prompt-based tool calling", where I
have an extra paragraph explaining to ollama how to extract filenames
from user text. This solution is fragile: as complexity grows, the LLM
is increasingly confused about what is going on, where the paramters
are.

Few-shot prompting, giving examples, is more effective.

Three Design Tasks
------------------
The following comes up:
* Can I generate sheaf sections from Atomese, i.e. create the IDL
  section for a dot-product, given the Atomese expression for the
  dot-product? (or rather, can I trick Claude into doing this?)
* I need to (initially, at least) pair the Atomese expression, e.g.
  for a dot product, with a verbal description. This pairing is already
  avaiable as a demo.scm file somewhere, but it is informal. A more
  direct, formalized pairing seems desirable ... but how?
* I need a way of composing jigsaws, mediating in English.  I can
  verify formal compositionality by running them through LG or perhaps
  some simpler formal system...

Clauding
--------
I wrote the below for Claude:

I have a demo file, called dot-product.scm  that is written to be an
example tutorial for human readers. It shows how to compute the dot
product of two vectors, in pure Atomese. It is annotated in such a way
as to explain exactly what is going on. But it is also entirely
stand-alone -- it inpcludes boilerplate to set things up, and print
statements that print to stdout -- it assumes that a human will be
cutting and pasting from that file to a guile REPL prompt.  I need this
converted to a lexical entry, which will be stored in the AtomSpace.
This lexical entry will consist of one or more blobs of text (a vector
of blobs of text!) that explain the Atomese for a dot-product.
Associated with this blob is the actual code for the dot product. Next,
there needs to be a precise jigsaw definition of the inputs and outputs,
and finally a blob of text that describes the jigsaw.

So this is a four-vector: a verbal description of the code, the code
itself, the IDL of the code, and a verbal description of the IDL. The
precise IDL exists because I have tools that can explicitly verify the
syntactic correctness of the attachment of any two connectors.

I will be asking a sophisticated LLM, such as you, Claude,  to help
create this four-vector. However, I would like to have a simpler system,
such as ollama, work with the actual assembly of the jigsaws into more
complex subsystems.  For this last part, this four-vector can be
extended with additional floating-point vectors that are embeddings of
the text.  This is the general idea. The overall design remains a bit
vague, as do the precise usage patterns.

And what did Claude say in response? "This is a genuinely novel
architecture". Fuck me. Every time I try something novel with Claude,
I get a ball of spaghetti code, and I am trying to figure out how to
avoid that, here.

Basically, Claude echoed back what I said above, then asked a bunch of
shallow, inane questions that reveal it does not understand the big
picture, but is quite eager to get lost in the details. That is how
spaghetti code is born: the urge to write code, before understanding teh
problem. Hmm.

Implementation
--------------
I will attempt to implement some variant of the above ideas in the
[agents project](https://github.com/opencog/agents). Wish me luck.
