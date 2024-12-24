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
the stream is just a `LinkValue`. I guess we could dress this up, and
call it a `LinkStream` but ... Hmmm.
