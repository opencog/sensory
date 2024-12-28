Recursion
=========
Observation is naturally recursive. When one looks, one typically sees
many items. Now what? Some of these need to be examined in greater
detail. This is "action selection", narrowed to the task of "looking".

The practical example here is of the recursive examination of a
directory tree. At each level in the tree, one obtains a listing of
items in that tree; one may then descend, or not, deeper into that tree.
How is this to be represented?

So, for example, performing `ls` on `/etc` and selecting for directories
only, once gets back
```
(LinkValue
  (StringValue "file:///etc/vim")
  (StringValue "file:///etc/opt")
  (StringValue "file:///etc/nvme")
  (StringValue "file:///etc/ldap")
  (StringValue "file:///etc/lvm"))
```
and so on.  In the current design, these can only be examined if
converted to `SensoryNode`. The conversion is easy; it can be done
by a `Filter` `Rule` that accepts sole StringValues as items, and rewrites
them with `StringOf`. One gets
```
(LinkValue
   (SensoryNode "file:///etc/vim")
   (SensoryNode "file:///etc/opt")
   (SensoryNode "file:///etc/nvme")
   (SensoryNode "file:///etc/ldap")
   (SensoryNode "file:///etc/lvm"))
```
Next, one wishes to apply `OpenLink` to each of these, so as to observe
what's in there. How should this be done? Some design and implementation
choices are pondered below.

Auto unwrapping
---------------
The easy (but wrong) design decision is to implement `OpenLink` so
that it automatically distributes over lists.  Thus, one design choice
is to have
```
(OpenLink (Type 'FileSysStream)
   (LinkValue
      (SensoryNode "file:///etc/vim")
      (SensoryNode "file:///etc/opt")
      (SensoryNode "file:///etc/nvme")
      (SensoryNode "file:///etc/ldap")
      (SensoryNode "file:///etc/lvm")))
```
to return
```
(LinkValue
   (FileSysStream)  ; with internal state "file:///etc/vim"
   (FileSysStream)  ; having internal state "file:///etc/opt"
   (FileSysStream)  ; etc.
)
```
Thus, if we imagine a `SensoryNode` to be an eyeball, and an open
sensory node to be an eye looking at a specific thing, then the above
has created a rather large number of eyeballs, and resursive descent
into the file system will create even more.  Its a bit of a fork-bomb.
It should probably not be built in as an automatic list iterator inside
of the `OpenLink`.

Filter rewriting
----------------
Fortunately, there already is a list iterator: the `FilterLink`.
It makes the default assumption that it is given a sequence of items,
and that the filter is to be applied to each item (sequentaily).

In this project, the filters are almost always going to be `RuleLink`s,
because they provide a reasonable way of specifying rewriting.
`RuleLink`s, when combined with a filter, have two parts. The input
pattern to a rule acts as a go/no-go decision maker: the rule is
applied, only if the pattern matches. Then, variables in the pattern
are grounded on the inputs, which are used in the rule's output
(rewrite) pattern.

For the problem proposed above, the acceptance pattern to the rue is
easy:
```
   (VariableNode "$item")
```
This will match anything at all.  The output rewrite is then
```
   (Open (Type 'FileSysStream)
      (StringOf (Type 'SensoryNode) (Variable "$item")))
```
Assuming the items are `StringValue`s (or `Node`s), the `StringOf`
will mint a single `SensoryNode` corresponding to that item. The
`Open` will then mint a `FileSysStream` value.  This produces the
desired result: a list of `StringValue`s (holding directory names)
have been converted into a list of open streams.

If the input item is not a node or string, the `StringOf` will throw an
exception. If the `SensoryNode` is not a directory, or it is a path
that is not in the file system, or if it is an invalidly-formatted URL,
then `OpenLink` will throw an exception.

We don't have an exception-handling architecture, yet. However,
exceptions do seem like the correct approach: the input pattern could
have performed validation on the input, but this just wastes CPU cycles
when the input is clean. Exceptions hide any manditory checking in
the C++ code implementing `OpenLink`, where CPU overhead is minimized.


Decision-Making
---------------
The Filter-Rule combination uses the Filter recognizer pattern to
perform a simple go/no-go decision: either the recognizer pattern is
matched, or it is not. What to do if additional decision-making is
desired? Some design choices below.

### Fancy rewrite patterns
Additional decision-making can be done directly in the rewrite pattern,
using the `CondLink`, for example. Thus,
```
   (Cond
      (Equal (Variable "$file-type") (Node "dir"))
      (Open (Variable "$file-name")))
```
assumes that there were two variables provided by the input pattern:
the file name and the file type; if the type is a dir, then the
directory is opened, else a null pointer is generated. I think the
`FilterLink` interprets null pointers as something to be ignored, and
just moves to the next item in the input. An alternative might be to
generate a `VoidValue`, but this should probably be reserved for when
actual void values are really needed for something.

### Decision Rules
An alternative is to invent a new kind of rule, that has an explicit
slot for a decision-maker. Currently, rules are of the form
```
(RuleLink
   <variable declarations>
   <input-match pattern>
   <output-rewrite pattern>)
```
The proposal is to define
```
(DecisionRuleLink
   <variable declarations>
   <input-match pattern>
   <evaluatable clause>
   <output-rewrite pattern>)
```
where the `<evaluatable clause>` is any Atom that is evaluatable, and
returns a TV that can ultimately be interpreted as true/false.

This does not seem to offer any advantage over the use of a `CondLink`,
shown above, and is in some ways worse: the `CondLink` can process a
sequence of conditionals, whereas the evaluatable clause delivers a
single yes/no decision.

One could arrange the evaluatable clause to return a boolean vector,
which would then be combined with a vector of output rewrite patterns.
But Atomese already has a collection of vector-processing Atoms, and
creating some new, sideways interpretation here seems wrong.

Parallelism
-----------
Short reminder: the `ExecuteThreadedLink` can take a sequence and run it
in multpile threads. This should be enough to dissipate an concerns
about serialization, sequencing and parallelism.

Recursion
---------
With the above out of the way, we can finally tackle recursion. How?
Given a stream anchored at an Atom-Key location, that stream can be
processed, and the results placed at that same Atom-Key location. Thus,
calling `cog-execute!` once goes one level deep in the recursion.
Several design issues arise:

* Instead of using a specific anchoring location, can there be some
  `FeedbackLink`  that explicitly wires the output back into the input?
  The short answer seems to be "no". Contemplating the implementation
  of such a link seems to necessitate some hidden, private variant
  of an anchor, and it we're going to do that, then why bother with the
  complexity of hiding it?

* There does seem to be a need for a `ConcatenateLink` of some kind,
  a way of flattening a list-of-lists, down to a single list. The need
  for this arises in the recursion step: a single item (e.g. the name
  of a directory) is turned into a list. A list of items becomes a list
  of lists. If the recursive filter is to be applied, this list of lists
  needs to be flattened. Are there other ways of doing this? Perhaps
  append? Maybe fold-list-nil? apply-append?

* Running `cog-execute!` once just takes one step in the processing.
  Atomese does not currently have any direct form of "do until done"
  structures, although this can be emulated using two techniques.
  There is a tail-call example in the main AtomSpace examples dir,
  and also a recursive pattern matching example. How to adaptt these
  to filter chains is not yet clear.

* Unix directory listings contain both `.` and `..` directories, and
  recursing on these will lead to infinite loops. How is this to be
  avoided?

* File permissions means that some files and directories will be
  unaccessible. This is handled by having the the `FileSysStream`
  throw exceptions for these cases. A design strategy is needed for
  excpetion handling. For now, having `WriteLink` try and catch seems
  like an OK solution.
