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
Another alternativ




Sequencing
----------
Here's the confusion. With a text stream, say, the IRC chat channel, or
the xterm TTY, once a connection is made, the flowing items are text,
and are explicitly handled line at a time. When some text shows up on an
IRC channel, it shows up, and you process it, else you wait for more.

Here, its recursive: the stream is a stream of sensory nodes.  Openening
each crates a new stream. Which must then be processed. Either in
serial, or in parallel. Lets do serial, first.

Do we need some kind of ProccessNode or ProcessLink or RunLink that,
when executed, runs the items in it's queue in serial or parallel?
That is, all examples until now used `(cog-execute! foo)` and it would
run the single thing `foo` which is typically just a single process
stream, of the form `(Write (Open ...) stuff)`. It seems that we now
need a `ForEachExecuteLink` to explicitly run each pending stream.

Atomese already as a `SequentialAndLink`, but it already has defined
semantics, and repurposing it for stream processing seems wrong.
Perhaps `ExecuteThreadedLink`, which has the desired semantics, I think?
Atomese is complicated. Who came up with this shit?

The problem with `ExecuteThreadedLink` is that it assumes a list or
sequence as it's argument. But here, what we really want to do is to
**apply** the `OpenLink` to each in a sequence.  Current Atomese does
not have an `ApplyLink`. Sigh.
