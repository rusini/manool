---
# specification/standard-library/composite-data-types.md
title:   Composite Data Types -- Specification
updated: 2019-12-22
---

{%include spec_header.md%}{%raw%}


In MANOOL an aggregate value (or object, depending on your point of view) incorporate other values (or objects, respectively). This normally happens
irrespective of aggregate element types and without placing restrictions on the number of elements on behalf of the aggregate type itself. Thus, for instance,
complex numbers are *not* aggregates, at least for the purposes of this section.

In MANOOL aggregate elements shall have some default order within that aggregate, and each aggregate element shall be associated with its key, which is either a
value of arbitrary type or an integer in the range 0 thru the number of elements minus 1 (called in this case an _index_). All aggregate types should allow for
partial updates in some way in an asymptotically efficient way.

Elements of an aggregate are sometimes also referred to as aggregate members. Note that in MANOOL it is impossible to construct an aggregate that incorporates
itself (due to non-referential semantics).

An aggregate view is an object (or value, depending on your point of view) that provides access to elements of an aggregate (real or imaginary) as though the
view itself were an indexed aggregate; that is, by providing the `Size` (number of elements) and `Apply` (access by index) polymorphic operations. No other
requirements are placed on aggregate views (in particular, the existence of update or non-default comparison operations is not required and normally not
needed); a proper indexed aggregate can be considered an aggregate view as well (providing access to its own elements).

Thus, an aggregate view may be
  * a proper indexed aggregate, which has its own storage for the elements;
  * an object that provides (on-demand) access to elements of some proper aggregate (or even a portion thereof with optional transformation applied on-demand);
    or
  * an object that provides access to a completely virtual (storage-less) aggregate, such as a Fibonacci sequence generated on-demand or an object that provides
    an aggregate-like view onto an (practically end-less) input stream or a relational database recordset.


{%endraw%}{%include spec_footer.md%}
