Structure Learning
==================
New set for notes concerning the learning of structure. Different
chapter than the earlier notes.

Start with a recap of the earlier proect for learning structure.

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
