Recursion
---------
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
converted to `SensoryNode`. The conversion is easy; one gets
```
(LinkValue
   (SensoryNode "file:///etc/vim")
   (SensoryNode "file:///etc/opt")
   (SensoryNode "file:///etc/nvme")
   (SensoryNode "file:///etc/ldap")
   (SensoryNode "file:///etc/lvm"))
```
Next, one wishes to apply `OpenLink` to each of these, so as to observe
what's in there. How should this be done?

The easy (but probably wrong) design decision is to allow `OpenLink` to
act on lists like these. Thus, one design choice is to allow
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
sensroy node to be an eye looking at a specific thing, then the above
has created a rather large number of eyeballs, and resursive descent
into the file system will create even more.  This is OK if we've got
multiple threads to do the work of looking, but probably wrong for a
single-threaded process, which needs to serialize observations.
But how?

Decision-Making
---------------
The next obvious answer is that each directory in the listing, i.e. the
`(StringValue "file:///etc/ldap")` should be passed to an examiner,
which makes a go/no-go decision for how to act next. A go decision then
results in a conversion of the `StringValue` to a `SensoryNode`, to
which the `OpenLink` is applied. The decision is then a stream of
directories to be examined. But this takes us back to where we started:
the stream is just a `LinkValue`.

This can be dressed up in one of several ways. One is to wrap it in
`LinkStreamValue` instead of `LinkValue`. The streams have the implicit
semantic interpretation of pesenting items in sequential order, i.e.
time-like order, whereas a plain `LinkValue` is merely a list.
That is, a Stream implicitly asks that you dequeue values one at a time,
and process them as individual units.

So far, this semantic is just implicit. The documentation encourages you
to think this way, without actually coming out and insisting upon it.
Perhaps it is time to come out? The alternaitve isw to invent yet
another `Value`, some `SerializedSequenceOfItemsValue`, which is
explicit about the need to serialize processing. But perhaps this is not
needed, and the existing collection of `Stream`s are enough for current
needs.

So, how does this actually work? Again, we've now got:
```
(Open (Type 'SomeSensoryType) (Stream item, item item ...))
```

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
