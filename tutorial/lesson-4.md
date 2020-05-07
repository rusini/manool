---
title:   Lesson 4
updated: 2019-10-16
---

{%include page_header.md%}{%raw%}


Aggregates
----------------------------------------------------------------------------------------------------------------------------------------------------------------

MANOOL provides a comprehensive set of composite data types, which differ in basic operations and internal data structure.

You can always iterate over elements of a composite in an easy way by using a `for` loop:

    { {extern "manool.org.18/std/0.5/all"} in
    : for { E = {array of "Red" "Orange" "Yellow" "Green" "Blue" "Indigo" "Violet"} } do
      Out.WriteLine[E]
    }

  (here we are constructing an _array_ out of elements by using the construct `{array of ...}`).

Output:

    Red
    Orange
    Yellow
    Green
    Blue
    Indigo
    Violet



In MANOOL an aggregate value (or object, depending on your point of view) incorporate other values (or objects, respectively). This normally happens
irrespective of aggregate element types and without placing restrictions on the number of elements on behalf of the aggregate type itself. Thus, for instance,
complex numbers are *not* aggregates, at least for the purposes of this section.

In MANOOL aggregate elements shall have some default order within that aggregate, and each aggregate element shall be associated with its key, which is either a
value of arbitrary type or an integer in the range 0 thru the number of elements minus 1 (called in this case an _index_). All aggregate types should allow for
partial updates in some way in an asymptotically efficient way.

Elements of an aggregate are sometimes also referred to as aggregate members. Note that in MANOOL it is impossible to construct an aggregate that incorporates
itself (due to non-referential semantics).

An aggregate view is an object (or value, depending on your point of view) that provides access to (real or imaginary) elements of an aggregate as though the
view were an indexed aggregate itself, that is by providing the Size (number of elements) and Apply (access-by-index) polymorphic operations. No other
requirements are placed on aggregate views (existence of update or non-trivial comparison operations is not required), and a proper indexed aggregate can be
considered an aggergate view as well, providing access the their own elements.


### Concatenation and Slicing
Examples:
~~~
{ {extern "manool.org.18/std/0.5/all"} in
: var { A = {array of "Red" "Orange" "Yellow" "Green" "Blue" "Indigo" "Violet"} } in
: for { E = A[Range[2; Size[A]]] + A[RevRange[0; Size[A] - 1]] } do
  Out.WriteLine[E]
}
~~~
~~~
{ {extern "manool.org.18/std/0.5/all"} in
: var { S = {sequence of "Red" "Orange" "Yellow" "Green" "Blue" "Indigo" "Violet"} }
  in
: var { SR = S[RevRange[Size[S]]] } in
: for { E = S![Range[2; Size[S]]] + SR![Range[1; Size[SR]]] /*O(1)*/} do
  Out.WriteLine[E]
}
~~~
Output:
~~~
Yellow
Green
Blue
Indigo
Violet
Indigo
Blue
Green
Yellow
Orange
Red
~~~

### Sets
Examples:
~~~
{ {extern "manool.org.18/std/0.5/all"} in
: for { E = {set of "Red" "Orange" "Yellow" "Green" "Blue" "Indigo" "Violet"} } do
  Out.WriteLine[E]
}
~~~
~~~
{ {extern "manool.org.18/std/0.5/all"} in
: for
  { E = {set of "Red" "Green" "Blue" "Indigo" "Violet"} +
    {set of "Red" "Yellow" "Orange" "Green" "Blue"}
  }
  do
  Out.WriteLine[E]
}
~~~
Output:
~~~
Blue
Green
Indigo
Orange
Red
Violet
Yellow
~~~

### Partial Updates and Iteration over Lazily Evaluated Slices
Example:
~~~
{ {extern "manool.org.18/std/0.5/all"} in
: var { A = {array of "Red" "Orange" "Yellow" "Green" "Blue" "Indigo"} } in
  A = A! | "Violet" | "Red" /*ammortized O(1)*/ -- append violet and red
  A[5] = "Dark Blue" /*O(1)*/ -- replace an element; shorthand for:
  /* A = A!.Repl[5; "Dark Blue"] */
: do Out.WriteLine[A[Size[A] - 1] /*O(1)*/] after -- last elem
: for { E = A.Elems[Range[Size[A] - 1]] /*O(1)*/ } do -- all but the last elem
  Out.Write[E; ", "]
}
~~~
Output:
~~~
Red, Orange, Yellow, Green, Blue, Dark Blue, Violet, Red
~~~
Example:
~~~
{ {extern "manool.org.18/std/0.5/all"} in
: var { S = {sequence of "Red" "Orange" "Yellow" "Green" "Blue" "Indigo"} } in
  S = {sequence of "Violet"} + S! | "Violet" /*O(1)*/ -- prepend and append violet
: do Out.WriteLine[S[Size[S] - 1] /*O(1)*/] after -- last elem
: for { E = S.Elems[Range[Size[S] - 1]] /*O(1)*/ } do -- all but the last elem
  Out.Write[E; ", "]
}
~~~
Output:
~~~
Violet, Red, Orange, Yellow, Green, Blue, Dark Blue, Violet
~~~
Example:
~~~
{ {extern "manool.org.18/std/0.5/all"} in
: var { S = {set of "Red" "Orange" "Yellow" "Green" "Blue" "Indigo"} } in
  S = S! | "Violet" | "Red" /*O(log N)*/ -- include violet and red (if not present)
  S["Orange"] = False /*O(log N)*/ -- exclude orange (if present); shorthand for:
  /* S = S!.Repl["Orange"; False] */
: do Out.WriteLine[S.Elems[][Size[S] - 1] /*O(1)*/] after -- last elem
: for { E = S.Elems[Range[Size[S] - 1]] /*O(1)*/ } do -- all but the last elem
  Out.Write[E; ", "]
}
~~~
Output:
~~~
Blue, Green, Indigo, Red, Violet, Yellow
~~~

### Nested Aggregates
Example:
~~~
{ {extern "manool.org.18/std/0.5/all"} in
: var { Tab = {array 9 of: array 7}$ } in
  Tab[0] = {array of "Red" "Orange" "Yellow" "Green" "Blue" "Indigo" "Violet"}$
  { for { I = Range[1; Size[Tab]] } do -- fill out the table
  : for { J = Range[7]$ } do
    Tab[I; J] = Tab[I - 1; (J - 1).Mod[7]] /*amortized O(1)*/ -- shorthand for:
    /* Tab[I][J] = Tab[I - 1][(J - 1).Mod[7]] */              -- (but faster)
  }
: for { I = Tab.Keys[] } do -- print the table
: for { J = Tab[I].Keys[] } do
  Out.Write[Tab[I; J] /*O(1)*/] -- shorthand for:
  /* Out.Write[Tab[I][J]] */    -- (but faster)
: if J < Tab[I].Size[] - 1 then
  Out.Write[" "]
  else
  Out.WriteLine[]
}
~~~
Output:
~~~
Red Orange Yellow Green Blue Indigo Violet
Violet Red Orange Yellow Green Blue Indigo
Indigo Violet Red Orange Yellow Green Blue
Blue Indigo Violet Red Orange Yellow Green
Green Blue Indigo Violet Red Orange Yellow
Yellow Green Blue Indigo Violet Red Orange
Orange Yellow Green Blue Indigo Violet Red
Red Orange Yellow Green Blue Indigo Violet
Violet Red Orange Yellow Green Blue Indigo
~~~

### Records
Example:
~~~
{ {extern "manool.org.18/std/0.5/all"} in
: for
  { E =
    { array of
      {record of Description = "Set A"; Op = {proc {A; _} as A}}
      {record of Description = "Set B"; Op = {proc {_; B} as B}}
      {record of Op = (+); Description = "Union"}
      {record of Op = (*); Description = "Intersection"}
      {record of Op = (-); Description = "Difference"}
      {record of Op = (/); Description = "Symmetric difference"}
    }$
  }
  do
: var
  { S = {set of "Red" "Green" "Blue" "Indigo" "Violet"}$.(E[Op])
    [{set of "Red" "Yellow" "Orange" "Green" "Blue"}$]
  }
  in
  Out.WriteLine["===== "; E[Description] /*O(1)*/; ":"]
: do Out.WriteLine[S.Elems[RangeExt[Size[S] - 1; 1]][0] /*O(1)*/] after -- last elem
: for { E = S.Elems[Range[Size[S] - 1]] /*O(1)*/ } do -- all but the last elem
  Out.Write[E; ", "]
}
~~~
Output:
~~~
===== Set A:
Blue, Green, Indigo, Red, Violet
===== Set B:
Blue, Green, Orange, Red, Yellow
===== Union:
Blue, Green, Indigo, Orange, Red, Violet, Yellow
===== Intersection:
Blue, Green, Red
===== Difference:
Indigo, Violet
===== Symmetric difference:
Indigo, Orange, Violet, Yellow
~~~

### Maps
Example:
~~~
{ {extern "manool.org.18/std/0.5/all"} in
: var
  { Es =
    { map of
      "Red"    = "Rojo"
      "Orange" = "Naranja"
      "Yellow" = "Amarillo"
      "Green"  = "Verde"
      "Blue"   = "Azul"
      "Indigo" = "Indigo"
      "Violet" = "Violeta"
    }
  }
  in
: var { Pt = Es } in        -- O(1)
  Pt["Red"]    = "Vermelho" -- O(N)
  Pt["Orange"] = "Laranja"  -- O(log N)
  Pt["Yellow"] = "Amarelo"  -- O(log N)
: let
  { WriteMap =
    { proc { S; M } as
    : for { K = M.Keys[]; E = M.Elems[] } do -- parallel iteration
      S.WriteLine[K; " -> "; E]
    }
  }
  in
  Out.WriteLine["=== In Spanish ==="];    Out.WriteMap[Es]
  Out.WriteLine["=== In Portuguese ==="]; Out.WriteMap[Pt]
}
~~~
Output:
~~~
=== In Spanish ===
Blue -> Azul
Green -> Verde
Indigo -> Indigo
Orange -> Naranja
Red -> Rojo
Violet -> Violeta
Yellow -> Amarillo
=== In Portuguese ===
Blue -> Azul
Green -> Verde
Indigo -> Indigo
Orange -> Laranja
Red -> Vermelho
Violet -> Violeta
Yellow -> Amarelo
~~~


{%endraw%}{%include page_footer.md%}
