Vector Similarity
-----------------
The previous file [DesignNotes-G](DesignNotes-G.md) pondered a general
problem of type convesion, neeed to deal with mildly incompatible
streams. It uses `EqualLink` as the base example.

A different variant on this problem is that of vector similarity.
Consider a search pattern that isolates a collection of vectors,
and one wishes to compare two vectors, using any one of the conventional
comparison measures: Hamming Distance, Jacquard similarity, Cosine
Distance, Mutual Information, Conditional Probability, etc. Given
two vectors, these each produce a single floating point number, or
possibly more than one.

The goal is to write an agent that can perform this compare. It's an
"agent" because it self-describes it's own input types, and its output
types as well: the format descriptions are not external to the agent,
but are intrinsic to it. That is, the agent presents a list of
connnectors that are joinable.
