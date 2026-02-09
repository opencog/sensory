
Ollama Wrapper
--------------
What the  hack? This is a cheap and cheesy wrapper that exposes Ollama
to an Atomese API. Its a hack. It's out-of-bounds and not entirely
acceptable, but ...  it will do for now, as a placeholder.

What's going wrong here? Well, the goal of the sensory projectwas/is to
explore how Atomese can interact with and manipulate the external world,
and to do so using some API that allows exploration and assembly.
Something tht embodies the spirit of shaves and jigsaws. And this, the
code here, is most certainly not that. What you have in front of you is
just a hokey FFI interface into Ollama. And the world does not need yet
another FFI. So overall everything currently in this directory is a huge
disappointment.

But I thought it would be good to have at least some kind of LLM API to
fool around with, and ... this fulfills that need. So we're going ahead
with this, for now. It does need a fundamental rethinking, though.
