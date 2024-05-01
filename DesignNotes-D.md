Design Notes Part D -- Parsing Implementation
=============================================
***Late April 2024***

OK, so Design Notes Part C sketched out a system for examining connector
sets, and using LG style parsing to arrive at candidate linkage
diagrams. This is followed by a step to select one linkage, and then a
step to perform the actual hookups. The result is, ideally, a single
network that, when executed, will run automatically.

The design has several unsolved issues.
* It required the specification of `MyFooAgent` which contains the
  needed connector sets for that Agent. The current LookatLink requires
  this to be a C++ class. Clearly, this is impractical.

* An automated way of making `CrossSection`s is needed. Probably a
  `CrossSection` toolkit. It has to be Atomese, of course.

* A `HookupLink` ...

* Clear up the connector sex semantics. It's messy, right now.
  Clear up flow data types, they're wonky: either strings or `ItemNode`s
  or whatever. String processing is biting us in the butt.

* The xterm demo creates hookups of streams running in inf loops,
  because `OutputStrem::do_write_out()` enters an inf loop when the
  source is a stream. The IRC demo does not run in an inf loop, because,
  after all the filtering, the code does not realize that there is a
  stream source at the far end, and as a result, each I/O has to be
  single-stepped. So which is it: automatic info loop, or
  single-stepping?
