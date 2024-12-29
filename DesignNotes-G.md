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
