Memory, Assembly, Understanding
===============================
Below follows some ruminations on a simple design that could provide a
verbal (text) interface to complex structural systems, as well as a
rudimentary form of memory. It is based on using an LLM, such as ollama,
to generate vector embeddings of text snippets; these vector embeddings
can then be associated with matching structural (symbolic) entities. It
might offer a simple way forward past some earlier design blockages
encountered in this project.

The perspective is that of "semantic routing": the LLM is treated as a
natural language API into non-verbal systems; the "thinking" happens
non-verbally, while the text LLM provides a way of manipulating,
controlling and working with the non-verbal elements. The idea is to
build on the concept of RAG (retrieval-augmented generation), semantic
search or semantic routing, by building triples of
```
   (text, vector-embedding-of-text, symbolic-structure).
```
This is a pre-MCP conception of the interface, in part because Ollama
does not have MCP support, and in part because MCP does not do what I
want it to do; and so I'm exploring some more basic technologies.


A "memory" architecture
-----------------------
The core architecture builds on the conventional notion of a
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
  pure-Atomese implementation of a vector DB, or (3) do both.

Option (3) above is the interesting one. This creates a pure Atomese
description of what a vector DB should do. In essence, the Atomese can
be thought of as pseudo-code, except that its a bit more precise, since
Atomese is directly executable (with questionable performance, as well
as having other issues, like an unclear API specification).

FWIW, the pure-Atomese version needs to also implement the search algo.
* IVF -- inverted file index: Assign the vectors to k-means clusters,
  then do dot-products against the clusters.
* HNSW -- "Hierarchical Navigable Small World" -- build a graph of
  nearest neighbors, and then hill-climb (i.e. "greedy") to find
  closest
* ColBERT -- vectorize tokens, sum max tokens (???)
* My old "membership club" idea from the learn project.


Off-the-shelf
-------------
Some existing systems:
* LangChain and LlamaIndex provide MCP-like API's
* Gorilla LLM -- specifically trained for tool calling.


Jigsaw API's
------------
At the center of the vector DB is the vector dot-product.
Mathematically, computationally, this is "trivial" -- its just a loop
that sums (accumulates) the product of two vectors. So that's
"conceptually easy". The "hard parts" are these:

* What is the correct notation for the pseudo-code for a dot-product?
  What do the symbols in that notation mean, and how do we symbolically
  express this idea?

A partial, but incomplete answer to the above is to express it in
Atomese. First, an explanation of why its an OK-ish answer, and then
why it fails, and is incomplete.

Expressing the dot product in Atomese is an OK-ish answer because there
already is a demo that does this. It not only provides a symbolic
expression for the dot product, but it also "runs". This is because
all Atomese has C++ as an underlying default implementation. Supply the
data, and say "execute" and bingo: the result is computed.

Another reason that the Atomese for the dot-product is an OK-ish answer
is that it is sufficiently symbolic to formally capture the idea of what
the words "dot-product" mean. The Atomese is entirely recognizable in
terms of what a school textbook might say about dot-products.  human can
look at th textbook, and the Atomese, and say "oh, yes, I see these
these are the same thing" (or do the same thing, or are different
representations for the same thing.)

However, the Atomese expression is also incomplete. For example, there
is no obvious way to convert the Atomese expression to some arbitrary
programming langauge: Java, python, rust, whatever. Although the Atomese
can be thought of as "pseudocode", there aren't any existing compilers
that will convert this expression into functional code.

There are at least two solutions to the above problem, neither of which
are entirely satsifying. One solution is to ask Claude code to read the
Atomese, and convert it to Java, python, rust... whatever. For
reasonably short Atomese snippets, and lots of hand-holding, and human
verification, and unit tests, this is possible. It's not at all
automatic; it requires human intervention and dilligence.

Another solution is to write a compiler, an Atomese-to-whichever-
language compiler.  This is doable. Such a compiler would be large,
complex, and quite the beast. This exposes a different issue: the "so
what" issue. So what if I have such a compiler? What would I do with it?
Who is going to write the Atomese that will be compiled down to some
existing software programming language?

There are some provisional answers to this "so what" question, but they
first thread through a different concern:

* What is the API to the lambda that implements the dot product?

This API is needed, because it needs to be converted to Java, python,
rust... Somewhere, there needs to be engineering documentation that says
"this code here implements the dot product. It is a function call that
takes to arguments that are arrays, and it returns a float." This
documentation needs to be adapted to Java, python, rust ... as
appropriate. Even within a single programming language, there are
choices: should python decorators be used? Should it be a stand-alone
function, or should it be a method on a class?

Part of what the API does is to define the calling convention: it
specifies the inputs and outputs. This interface definition is needed,
if the function is to be used in some more complex algorithm.

For Atomese, the notion of jigsaws and connectors provides the needed
device. The input connectors define what the inputs are; the output
connectors define the outputs. This allows algos to be defined that know
how to hook up connectors. There is a rich variety of such algos; the
Link Grammar infrastructure is one; the odometer (in the 'generate'
github project) is another. More can be invented & created. The Viterbi
algo is an OK approach, although it has trouble scaling.

A prototype for such a connector-description infrastructure can be found
in the `sensory-v0` subdirectory of this project. This prototype exposed
another issue:

* Hand-writing IDL's as jigsaws is untenable. Each jigsaw will usually
  have three or more connectors. Each might offer several kinds of
  connectivity options. The description is verbose, and writing it is
  error-prone. And that is for just one function, say, the dot-product.
  How are hundreds or thousands of these to be handled?

That is, how is the lexis (of connectable functions) to be created?
How will it be managed?

The proposed but foggy answer to the above is that the lexis can be
maintained, managed and controlled by a textual LLM interface

created a

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
is increasingly confused about what is going on, where the parameters
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
  available as a demo.scm file somewhere, but it is informal. A more
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
stand-alone -- it includes boilerplate to set things up, and print
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
spaghetti code is born: the urge to write code, before understanding the
problem. Hmm.

Implementation
--------------
I will attempt to implement some variant of the above ideas in the
[agents project](https://github.com/opencog/agents). Wish me luck.
