Watching and Following
----------------------
Working with a "real world" user of the file/directory interfaces.
Two needs arise: the ability to tail a file as it is being written by
some external writer, and the ability to watch directory contents, as
they are being manipulted by some external agent.

I gave these tasks to Clause, which seems to be coding up what I asked
for in a literal manner, but Claude is mindless and unthinking, and
like the wish-granting genie, isn't quite delivering what was wanted.

What's the issue? Well, there are several.

My original implementation provides this implementation for streaming
reads:
```
ValuePtr StreamNode::stream(void) const {  
   return createReadStream(get_handle());
}
```
In this implementation, `this` holds the handle to the thing being
streamed, and `ReadStream` plucks out one item at a time from the object
being read. This provides a reasonable implementation for those classes
that do not provide thier own implementation.

Classes that do provide thier own implementation typically return a
ContainerValue (QueueValue or UnisetValue). The ReadStream is a plain
LinkValue. So one minor question mark: Should it have been a
ContainerValue instead? Right now, that extra complexity seems
un-needed.

The issue (and this is my fault) is that I asked (allowed) Claude to add
a new method, `follow()`, to the Sensory API. As I write this, I realize
that it should just have been `stream()`. I guess this was a late-night
thinko. I will go correct this now.

The other is typical Claudeness. The tailing is done by using inotify.
But the inotify code is placed straight in, without modularization;
Claude is now inserting very nearly duplicate cut-n-paste into the
directory watcher. So this needs re-wrapping.

These are "minor" design issues, and are easily correctable at this
point. And yet, they reveal a "fluid design boundary": adding new
methods called `follow()` and `watch()` feels entirely natural. The
names reflect the functions they will provide, and the functions are
generic enouch that one imagines there will be other sensory nodes that
will want to provide these functions. These could have been accepted
into the C++ code. But they would cause down-stream issues. Right?

The current streaming design assumes everything flows through the
`stream()` method. This makes sense: after sending the `*-open-*`
message to an object, its reasonable to assume that what one gets back
is something that can stream data. This is what Values were originally
imagined to provide, when first implemented eight years ago: this is
what the `update()` method was envisioned for. But now, if we add the
`follow()` and `watch()` methods, there's an ambiguity: which stream do
you want to follow? How does the wiring diagram work? Ugh. The knee-jerk
response is that this all needs to be collapsed down to one: `stream()`.
Perhaps some future experiences will require something fancier, but the
what and how of this is opaque.

This fluidity of design choices comes very easily to Claude: it's happy
to do anything. Any pile of shit that vaguely matches what you asked
for, Claude will generate this. Humans are a little bit better; some of
the more obviously flawed design choices are avoided. But humans do this
only one abstraction layer up; complexity clouds the issue. Look at the
evolution of Linux boot-loaders. One turns into several, the several
compete, offering different functions and levels of hardware
compatibility. They did not spring "fully formed" from the mind of the
author; each new bootloader was created by some developer who thought
"I can do this better than that one", and they could, up to a point.
Up to that point where software complexity again becomes hard to
control, and the future becomes hard to imagine.

This story repeats everywhere: we have dozens or hundreds of programming
languages, not one. Many apps for doing almost the same things. The
conventional justification is to draw an analogy to evolution: there is
a rich variety of creatures, because, in nature, there is a vast choice
of possible design alternatives. At each fork, nature tried both. Some
design choices really do end up better than others; some are about
equal.  But an organism consists of more than just just one design
choice: they are a sequence of many. Some of these paths are confluent;
biologists call this "convergent evolution".

But what happens if you are smart? Well, you can look something over and
decide: "gee, that is a stupid way of doing things; this is much
better". As an engineer, you can run mini-simulations in your mind,
quickly determine that some of the choices are just losers, and avoid
those. This short-circuits evolutionary search. The point that I am
making here is that the human ability to engineer is limited: we can
scale up to a certain complexity level, but no more. Above this
complexity level, it looks like evolution: every possible engineering
design choice is explored, more or less randomly, by hundreds of
different engineers.

That is, the fluidity of design choices invites ergodic exploration. In
nature, in Darwian Natural Selection, the exploration of design choices
is ergodic. Everything is tried. There is no "intelligent design" in
natural selection. Human engineers can apply intelligence, cutting off,
drying out some of these design choices. Assigning them a measure of
zero. So, what does intelligence do? It takes something that is ergodic,
and says, "nah, we care going to prune some of these explorations.
Everything that lies behind some certain branch point, it will be give a
probability of zero; that side of the branch point will not be explored.
That possible world of design choices will be made such that it never
occurs, i.e. corresponds to a probability measure of zero.

Is this what intellgence is? Hmm. But I digress. I need to copy above to
my diary.

Oh, but the other point I'm making is that human intelligence cannot
reach arbitrarily high up the complexity chain. We can see things only
up to some certainly level of complexity; above that is a threshold, a
phase transition: stuff above is ergodically explored; stuff below can
be engineered. That this level depends on intelligence is made clear by
Claude: it reaches the ergodic level that is lower than where I am. It
will happily propose crappy solutions that I have to weed away. But I
myself am limited: this design diary is an exploration of the design
choices in front of me, at the edge of the complexity level that I
personally can deal with. Something smarter than me can presumably cut
through some of the bullshit I've had to muddle through, here.

But this then raises both upward and downward problems. The downward
problem is easier to explain: what if I made a poor design choice very
early on, and baked it into my system: e.g. choosing Java instead of C++
as the project language. Can evolution be confluent? Yes; things would
be different, but could end up in the same place. This is kind of like
comparing Turing machines to register machines to RAM machines: they all
can, in prinicple, compute the same things, but have exponentially
different demands on time and space. That's hardware; analogous
statements can be said for algorithms: some algorithms might compute the
same things as others, but are exponentially faster/slower.

The upward problem is that most of activity of coding is tedious,
mindless, boring, repetitive. When I port code from here to there, or
add this feture or that one, most of it is stupid and time wasting.
Type `make` at the command line. Oh, failed to compile? Fix that. Do it
again and again and again. Just because I forgot to move some file, copy
some code, add a semi-colon to the end of a line. Claude releives much
of this tedium, and, for this, I am grateful: it can do the tedious bits
that I don't want to. It can do those tedious bits in a machine-like,
mechanical fasion. I am also grateful for jack-hammers, when I am
busting concrete. Plain-old chisel and hammer is not fun. So it appears
that "the world below" is best treated by mindless, mechanical
rearrangemenets. But "the world above" is impenetrable, unforeseeable,
and we have to ergodically try all possibilities. So my intelligence
exists at this phase-transition boundary, between what is machine-like
and tedious, below, and the unknowable, revealed through epiphanies,
above.
