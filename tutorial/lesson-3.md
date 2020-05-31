---
title:   Lesson 3 -- Tutorial
updated: 2020-05-31
---

<aside markdown="1" class="right">
Lesson 3 of 5 -- Tutorial
=========================
+ [Lesson 1](lesson-1#start)
+ **[Previous Lesson](lesson-2#start)**
+ **[Next Lesson](lesson-4#start)**
+ [Lesson 5](lesson-5#start)
</aside>

{%include page_header.md%}{%raw%}


Symbol Data Type
----------------------------------------------------------------------------------------------------------------------------------------------------------------

Symbol is one of the most fundamental and versatile data types in MANOOL. `extern`, `in`, `WriteLine`, `Out`, `+`, etc. are all symbols.

<aside markdown="1">
Although symbols resemble strings (with a few limitations), they differ in basic operations and internal data structure and constitute a disjoint data type due
to performance considerations. Converting values from String to Symbol may be slow, whereas comparing symbols and looking up entries in tables using symbols as
keys is always fast (and is guaranteed to complete in constant time).
</aside>

If an expression in MANOOL consists of a literal symbol, by default that symbol denotes itself (e.g. `WriteLine`, `(+)`, `Foo`) unless it starts with a
lowercase letter (e.g. `then`, `do`) or is bound to some other entity in the current scope (e.g. `Out`, `extern`, `if`, `(&)`). In the later case, the symbol
denotes that entity.

Here's a quick test code:

    {{extern "manool.org.18/std/0.5/all"} in Out.WriteLine[Foo ", " Foo == "Foo" ", " Foo.IsSym[]]}

Output:

    Foo, False, True

Values of type Symbol can identify
  * polymorphic operations (e.g. `WriteLine`, `(+)`, `(~)`, `Div`),[^a1]
  * record components and object attributes, and
  * any states, modes, options, tags, etc. you would like to represent by a simple identifier.

[^a1]: Moreover, symbols *are* polymorphic operations, which are directly applied to arguments.

To force literal interpretation of a syntactic construct in MANOOL, use the postfix operator `'` (single quote):

    { {extern "manool.org.18/std/0.5/all"} in
      Out.WriteLine[Foo' ", " Out' ", " extern' ", " if' ", " (&)' ", " then']
      Out.WriteLine[Out.WriteLine["Hello, world!"]']
    }

Output:[^a2]

    Foo, Out, extern, if, &, then
    value/object

[^a2]: The second expression tries to display a _syntactic list_ object (more on this later), and the operation `Str` returns a generic `"value/object"` for
       most data types in MANOOL.

A symbol can also be dynamically constructed from a string by using the constructor `MakeSym` and converted back to String with the operation `Str`:

    {{extern "manool.org.18/std/0.5/all"} in Out.WriteLine[MakeSym["Foo"] ", " MakeSym["Foo"] == Foo]}

Output:

    Foo, True

---

**Caution!!! Work in progress!!!**

---


Numbers Beyond Integers
----------------------------------------------------------------------------------------------------------------------------------------------------------------

You have full control over precise semantics of arithmetic operations in MANOOL. To cover a variety of practical needs and situations, the language provides
several numeric data types to choose from, such as Integer, several kinds of Floating Point with binary and decimal base, Complex, and Unsigned. In most cases,
you can expect MANOOL programs to give you reproducible numeric results regardless of peculiarities of your language implementation.

{:comment}
The MANOOL specification places explicit requirements on precise semantics of arithmetic operations in the language; thus, MANOOL should have in most cases a
completely reproducible behavior in this respect regardless of the implementation details.
{:/comment}

In the following examples we are going to learn about how to perform operations on fractional numbers, but first let's learn one minor but important feature:

    { {extern "manool.org.18/std/0.5/all"} in
      Out.WriteLine[{do Out.WriteLine["One"]; 2 + 3}]  -- do - explicit sequencing
      Out.WriteLine[{do Out.WriteLine["Two"]; 5 - 2}$] -- $  - compile-time evaluation
    }

Output:

    Two
    One
    5
    3

Example:

    { {extern "manool.org.18/std/0.5/all"} in
      -- Scientific-oriented arithmetic (base-2 internal representation)
      Out.WriteLine["one quarter (F64)  = " F64[".25"]$  " = " (F64[1] / F64[4])$]  -- precise representation
      Out.WriteLine["one tenth   (F64)  = " F64["1e-1"]$ " = " (F64[1] / F64[10])$] -- approximate representation
      Out.WriteLine["one third   (F64)  = " (F64[1] / F64[3])$] -- periodic fraction (when represented in decimal form)
      Out.WriteLine["one third   (F32)  = " (F32[1] / F32[3])$] -- ditto
      -- Human-oriented arithmetic (base-10 internal representation)
      Out.WriteLine["one tenth   (D64)  = " D64["1e-1"]$ " = " D64[".10"]$]
      Out.WriteLine["one third   (D64)  = " (D64[1] / D64[3])$]
      Out.WriteLine["one third   (D128) = " (D128[1] / D128[3])$]
    }

Output:

    one quarter (F64)  = 2.5000000000000000e-01 = 2.5000000000000000e-01
    one tenth   (F64)  = 1.0000000000000001e-01 = 1.0000000000000001e-01
    one third   (F64)  = 3.3333333333333331e-01
    one third   (F32)  = 3.33333343e-01
    one tenth   (D64)  = 0.1 = 0.10
    one third   (D64)  = 0.3333333333333333
    one third   (D128) = 0.3333333333333333333333333333333333

### Decimal rounding ###################################################################################################

MANOOL supports two rounding modes for decimal floating-point arithmetic:
  * round to the nearest, break ties to even (also known as Bankers' rounding), and
  * round to the nearest, break ties away from zero (also known as commercial rounding).

  Bankers' rounding is suitable for lengthy calculations where rounding errors should be avoided as much as possible, whereas commercial rounding is more
wide-spread in many cultures.

MANOOL provides two groups of decimal floating-point data types, one for performing arithmetic using the Bankers' rounding mode and another for using
commercial rounding:

    { {extern "manool.org.18/std/0.5/all"} in
      Out.WriteLine["Banker's rounding: " D64["2.000000000000021"]$ / D64[2]$ ", " D64["2.000000000000031"]$ / D64[2]$
      ", " D64[".25"]$.Quantize[D64[".0"]$] ", " D64[".35"]$.Quantize[D64[".0"]$]]
      Out.WriteLine["Common rounding:   " C64["2.000000000000021"]$ / C64[2]$ ", " C64["2.000000000000031"]$ / C64[2]$
      ", " C64[".25"]$.Quantize[C64[".0"]$] ", " C64[".35"]$.Quantize[C64[".0"]$]]
    }

Output:

    Banker's rounding: 1.000000000000010, 1.000000000000016, 0.2, 0.4
    Common rounding:   1.000000000000011, 1.000000000000016, 0.3, 0.4

### Signed zeros #######################################################################################################

MANOOL supports signed zeros in accordance with the specification IEEE-754 (only for binary floating-point arithmetic):

    { {extern "manool.org.18/std/0.5/all"} in
      -- Signed zeros supported
      Out.WriteLine[F64["1e-300"]$ / F64["1e300"]$ ", " ~F64["1e-300"]$ / F64["1e300"]$]
      Out.WriteLine[F64["+0"]$ ", " F64["-0"]$]
      Out.WriteLine[F64["+0"]$ == F64["-0"]$]
      Out.WriteLine[Atan[F64[0]$; F64["+0"]$]]
      Out.WriteLine[Atan[F64[0]$; F64["-0"]$]]
      -- Signed zeros unsupported
      Out.WriteLine[D64["1e-300"]$ / D64["1e300"]$ ", " ~D64["1e-300"]$ / D64["1e300"]$]
      Out.WriteLine[D64["+0"]$ ", " D64["-0"]$]
    }

Output:

    0.0000000000000000e+00, -0.0000000000000000e+00
    0.0000000000000000e+00, -0.0000000000000000e+00
    True
    0.0000000000000000e+00
    3.1415926535897931e+00
    0e-398, 0e-398
    0, 0


* * * * *
**[Continue](lesson-4 "Next Lesson")**
<aside markdown="1" class="right">
Lesson 3 of 5 -- Tutorial
=========================
+ [Lesson 1](lesson-1#start)
+ **[Previous Lesson](lesson-2#start)**
+ **[Next Lesson](lesson-4#start)**
+ [Lesson 5](lesson-5#start)
</aside>

{%endraw%}{%include page_footer.md%}
