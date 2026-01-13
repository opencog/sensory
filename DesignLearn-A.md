Structure Learning
==================
New set for notes concerning the learning of structure. Different
chapter than the earlier notes.

Start with a recap of the earlier project for learning structure.

Recap of Previous Learn Project
-------------------------------
The first attempt at structure learning is the github project
https://github.com/opencog/learn and perhaps this diary entry belongs
there. However, I want to incorporate the new ideas here, and so, for
now, this chapter is here.

The language learning started with computing pair-wise MI between words,
then uusing this to create a network between words, then breaking up the
network into jigsaws. The jigsaws were then clustered, and the clusters
then used to parse the same text, again. This all worked fine. The next
step of the plan was to then repeat: find correlations between these
clustered jigsaws, then tile again, etc. ad ininfinitum. That step was
never taken, for two reasons:

* Managing the batch processing pipeline became overwhelming. The
  propsoed solution to that was to create sensori-motor agents that
  could crawl the data "of their own volition"; or rather, automated
  in such a way that I could aim agents at the data and have them
  process and digest without my explicit intervention at a low level.
  In particular: no more shell scripts, no more perl scripts; it would
  all be Atomese. Solving that problem give rise to this github repo
  that this design file is in right now.

* The recursive step of "repeat and do it again" stumbled on the design
  of the "matrix" code base. (Split out of the atomspace repo and now
  located at https://github.com/opencog/matrix).  That code-base was
  written to be quite general, so that it could handle generic
  structures. The problem was that it was written in scheme. That means
  it integrated poorly and awkwardly with Atomese, and did not integrate
  at all with a sensori-motor design.

  The solution to that stumbling block was to develop the concept of
  "flows" in Atomese: of flowng data that travels through networks
  defined declaratively in Atomese. This required lots of low-level
  grunt-work: defining Atoms that could add, multiply, accumulate,
  take dot-products, mask bits, transpose columns and tows, but also
  could stream data, and also sort data... Lots of low-level grunt work,
  mostly completed.

So given that the primary issues were resolved in the two bullet points
above, I am now ready to try again. And short-term plans include working
with both audio and visual data, this time around. Concrete plans
further below.  But, before starting, some critiques, some planning,
some day-dreaming. The below will ramble a bit, because I haven't
figured out some parts yet.

Critique of Flow Design
-----------------------
The "flow" code and demos in the atomspace work "just fine", more or
less. However, several big issues exposed themselves during development.
Consider the problem of implementing the flow pipelines on GPU's. Viz.
I want to have the declarative description to be in Atomese, but the
processing to happen on the GPU. But how? Several answers, none
satisfactory.

A) Hard code GPU variants of `PlusLink`, `TimesLink`, etc. Yuck.  Hard
   work, fragile, non-portable. Any such implementation would become a
   some weird cross-over between interpreter and compiler. See below.

B) Write a compiler. Here, The `PlusLink` etc. become "assembly code"
   and the compiler turns it into CUDA code or OpenCL or whatever. Yuck.
   Such a compiler is large, complex, non-portable. Its also a "been
   there, done that" project: people have been writing compilers for
   fifty years, and doing it yet again for a highly specialized use case
   is boring and pointless.

C) Define the GPU hardware in such a way that it can be "perceived" by a
   sensori-motor agent that could "just look at it" and "use it". Yes!
   Great! A bit too pie-in-the-sky for now; I lack the infrastructure to
   get off the ground on this. It's an aspirational.

Status: Punt. Item C) is the aspirational goal. The code located at the
https://github.com/opencog/atomese-simd directory is some pre-prototype
of A) and B) and I might yet tinker with it some more. I do not yet have
an firm road-map for getting to C).

What is Structure?
------------------
I wish to define "structure" in some quasi-model-theoretic terms.
"Quasi" because the goal is not to be slavish to the formal textbook
definition, but to convert it into an artistic, sculptural impression.

Structures consist of axioms, rules, n-ary function terms, relations.
Formally, they also include the bool logical operators, but I want to
convert those into yet more functional terms. Why? Lets take a detour
into proof theory.

Proof theory works with axioms and inference rules. We have some
collection of axioms, asserted a priori, and some other collection of
inference rules, also asserted a priori. A theorem prover chains these,
forward or backward, to find "proofs" for "theorems". Instead, I want to
think of this process as being a kind of generation, where rules and
axioms are specific building blocks that are assembled.

The rules and axioms are jigsaw pieces; theorem proving is the assembly
of these. If it is free an unconstrained, then it is generation; if its
constrained, then some sort of satisfiability solver is needed: the
jigsaw much assmble so as to build be bridge between constraints. The
satisfiability algorithm must be fast, but also the bridge that is build
mst be minimal or optimal in some way.

### Minimization
The words "minimal" and "optimal" make me personally think of minimal
surfaces: soap bubbles, for instance. Geodesics in Riemannian geometry.
Assemblies of jigsaws do not have smooth differentiable structure, but
still, some of the ideas carry over analogously, because there are
several natural ideas of "minimal" for jigaw assemblies. For example:
the assembly that uses the minimal number of pieces. Or, if we weight
each piece with e.g. log probability (i.e. "entropy" or "information")
then minimize the sum of that.

### Homtopic deformation
The general "feel" of continutity is taken up by the Scott topology,
where we can talk about the homotopic deformation of jigsaw aassemblies.
There is a very easy way of understanding this if you are a software
programmer: a "homotopic deformation" is a "refactoring of the
software". Any collection of small changes that can be made to a
software program that does not alter it's function. Move if-statements
out of loops. Unroll loops. Stuff like that. These are homotopic
transformations: they alter the structure without altering function,
and, when taken as small, tiny changes, can be thought of as providing
a kind of "continuous deformation".

Consider an optimizer inside a compiler: it has some assembly snippet,
using some registers and some insns, and it can re-arrange some of the
insns, replacing them with others, choosing different register
assignments, thus creating more efficient, compact code without altering
its function. The `-O2` flag on a C compiler. The optimizer does not
work on bare assembly, but in abstract insn sets, the "intermediate
langauge" being a way-point.

But this analogy can be pushed "all the way": A compiler is a homotopic
defomration of a C/C++ program into assembly code; an assembler is a
further deformation (and linking!) into machine binary. A bridge between
two different represenations that does not alter the function: Whatever
the programmer wrote the C/C++ code to do, the machine binary will do
the same thing. The transformation from one to the other was
accomplished in a very long sequence of small steps.

### Differentiability
What about differentiability? Even that has an analog: finite
differences. When collections of jigsaws can be assigned a number (e.g.
total number of pieces; total weight; length of shortest path; total
number of connections/connectors; width (fatness) of necks, or of
waists... etc.) These may vary be discrete amounts, but differences can
be taken, giving an analogy to differentiation.

Can one write a finite-difference version of the Euler-Lagrange
equations (the solution of which defines geodesics)? I don't know.
Perhaps. People already run around claiming that one should maximize
free energy (Gibbs free energy) or perhaps the "integrated information";
all of these ideas imply some kind of differentiable stucture minimized
or maximized via some Euler-Lagrange-style differentail or difference
eqns. I don't want to get hung up on the details, but rather the
artistic impression: when there is stuff to be minimized and maximzed,
and when one has paths that can take you from here to there in some
quasi-continuous fashion, then one has some local difference-type
gradient that helps figure out how to move.

### What is a priori?
The above arguments is slippery and misleading.  Starting from "nothing"
we arrive at some very complex and arcane notions derived by analogy
from a huge body of work in mathematical physics to conclude "ah ha!
Free energy! That's the Answer with a Capital A!"

### Self-assembly
But I don't want this. I want a system to self-assemble (necessairly
a system that has moved to a critical point.) The word "critical" in
the sense of self-assembly and avalanches for sand-piles at criticality,
but now, for jigsaw pieces, not sand-grains.

The jigsaws are generalizations of structural parts: jigsaws are the
connectable parts of axioms, of inference rules, of lambda expressions,
of function terms, of relation terms, of logical connectives.

I want, and I think it is possible, to have a system that self-reifies:
a self-assembly of pieces that can figure out how to reason, deduce,
prove, use logic and discover axioms. A system that can perceive
structure, in a "sensori-" way, and can manipulate structure, in a
"motor" kind of way. So a sensori-motor system that percives structure,
and acts on, manipulates that structure.

### Perceiving structure
So again, what is structure, and how is it perceived? Once again: it is
the pairwise relationships or correlations between pairs of "things".
The pair-wise connections form a network graph, of vertexes and edges.
This network graph is in rough correspondence with the "external world"
that is being perceived. A "world model" of sorts.

Fix a vertex, and cut all the edges connecting to it in half. This gives
you a jigsaw piece. For each jigsaw piece; one for each vertex.
Categorize these: Many will be identical. The frequency (count) of
identical ones follows a Zipf power law distribution (as experimentally
observed, i.e. "in practice")

Now, cluster them "by similarity". One of the easiest ways fo doing this
is by cosine-similarity. Cosine similarity requires vectors; where do
the vectors come from?  They arise entirely naturally, in many ways. For
example, consider all jigsaws that have the label "A" for the center
vertex; these will have a large variety of labels "p,q,r..." on the
connectors. How many are there? Say there are `N(A | p,q,r...)` of
these. If we hold "A" fixed, while allowing "p,q,r..." to vary, this
forms a vector!  The ket `|p,q,r,...)` is the basis elt of the vector.
Suppose the central vertex is labelled by "B": then `N(B | p,q,r...)`
is another vector, and the dot product is easy:
```
    A . B = Sum_{p,q,r..}  N(A | p,q,r...) N(B | p,q,r...)
```
In practice, (i.e. experimentally measured) the dot products are
gaussian distributed, i.e. the data is distributed uniformly on a very
high dimensional sphere.




pairs.
