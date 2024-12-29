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
