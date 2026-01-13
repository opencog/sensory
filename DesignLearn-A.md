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

* The matri
