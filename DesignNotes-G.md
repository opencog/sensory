Flow Type Conversions
---------------------
There's an open problem of converting types between connections.
Type conversion is already supported in Atomese, with `FilterLink`,
`CollectionOfLink`, `StringOfLink` and more. So it works, in principle.
In practice, its ungainly.

### Example
To keep things focused, here's a concrete example. A terminal is opened,
a single line is read, and a string compare is done, to see if the user
typed in "xxx" (followed by newline).
```
(define term-loc (ValueOf (Anchor "crawler") (Predicate "term")))
(cog-execute!
	(SetValue (Anchor "crawler") (Predicate "term")
	(Open (Type 'TerminalStream))))

(cog-evaluate!
	(Equal input-loc (LinkSignature (Type 'LinkValue) (Item "xxx\n"))))
```

So what's the problem? Well, today, the terminal returns `ItemNode`
for each input. This is more-or-less wrong, or counter-productive
to the idea of flowing streams: instead, a `StringValue` should have
been returned.

Well, we can fix that easily enough, but this wrecks the readability of
the equality-compare. It would now have to be
```
	(Equal input-loc
		(LinkSignature (Type 'LinkValue)
			(StringOf (Type 'StringValue) (Item "xxx\n"))))
```
which took a comprehensible equality-compare, and made it just a bit more
opaque. This is irksome.

### Design Alternative A
Change the `TerminalStream` to have a configurable output type. So,
perhaps, like so:
```
	(Open
		(Type 'TerminalStream)
		(List (Item "output-type") (Type 'StringValue)))
```
This opens the terminal, passing a configuration parameter giving the
output type to present. The aobve style looks very much like a
conventional unix command-line-plus-args situation: some variable number
of config parameters are passed as "options" or "flags".

Modes could also be changed at run-time:
```
	(Write term-loc
		(List (Item "output-type") (Type 'StringValue)))
```
Run-time reconfig is dangerous, for obvious reasons.

### Design Alternative B
Auto type conversion during stream contents compare. That is, provide
a `StreamEqaulLink` that is like the `EqualLink`, but unwraps the stream
and then does some amout of automatic type conversion.

This solves two issues at once.  First, the verbose
```
	(Equal input-loc (LinkSignature (Type 'LinkValue) (Item "xxx\n")))
```
by the more svelte
```
	(StreamEqual input-loc (Item "xxx\n"))
```
The issue here is that `input-loc` has the form
```
(TerminalStream (stuff ...))
```
with `TerminalStream` being of type `LinkStream` which is of type
`LinkValue`, and so before any equality compare can even be started,
the stream itself has to be unwrapped. So automating this unwrap makes
Atomese decision/control expressions a bit more usable.

### Conclusion
Do both. It seems inevitable that streams will need to get configuration
and control parameters from somewhere, so Alternative A will ultimately
predominate, I guess. But Alternative A adds complexity and code and
makes wiring even harder than it already is. So its not the go-to
aternative.

Alternative B adds yet another Atom, as if we didn't have enough of
these. But it's small, simple, direct, and works for everyone, not just
terminal streams. So I guess that's the go-to, to keep things simple.
