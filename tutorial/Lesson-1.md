---
title:   Lesson 1
updated: 2020-04-24
---

{%include page_header.md%}{%raw%}


Hello World
----------------------------------------------------------------------------------------------------------------------------------------------------------------

The following is a traditional "Hello World" program in MANOOL:

    -- in "applicative" notation
    {{extern "manool.org.18/std/0.5/all"} in WriteLine[Out; "Hello, world!"]}

To play with MANOOL examples, you can use the [online evaluator] or run them from the command line according to the [instructions]. Here is just a couple of
ideas about how to run MANOOL programs from the command line:

    mnlexec hello.mnl

  (assuming you have placed your source code into `hello.mnl`) or, for short scripts:

    mnlexec <(echo $'{{extern "manool.org.18/std/0.5/all"} in WriteLine[Out; "Hello, world!"]}')

  (we use `$'...'` with a leading `$` here just to be sure we can more easily escape `'` characters in Bash in the future).

[online evaluator]: /eval     "MANOOL Online Evaluator"
[instructions]:     /download "How to Download and Install MANOOL"

The expected output is unsurprisingly

    Hello, world!

Using the online evaluator is convenient but limits you to just one source file. Anyway, consult [How to Download and Install MANOOL][instructions] for more
tips.

#### How does it work?

1. The construct `WriteLine[Out; "..."]` resembles a *function call* in many languages where the construct `WriteLine` would specify a function, and `Out` and
   `"Hello, world!"` would be the arguments.[^a1] Here, as a side effect of an evaluation of this expression, the phrase `Hello, world!` specified by a string
   literal eventually appears on the standard output specified by the argument `Out`.

2. During compilation of the whole expression `{{extern "..."} in ...}`, all identifier definitions[^a2] (including, in particular, the one for `Out`) from the
   standard library module specified by the string literal `"manool.org.18/..."` are *imported* into the scope that follows the keyword `in`.

[^a1]: In MANOOL ordinary functions are called _procedures_, and the construct `WriteLine[Out; "..."]` is more generally called an _applicative expression_
       (more on this later).

[^a2]: ...or rather their analogs in MANOOL called _bindings_...

<aside markdown="1">
This has nothing special by now and is quite a norm for a lot of languages nowadays, and I promise all of this is going to become more interesting later.
However, you may note that the main difference is in the (surface) syntax and the unusual choice of brackets and delimiters. Please bear in mind for now that
this is not an arbitrary choice nor a matter of personal preference but rather a result of many design trade-offs in the language.
</aside>

### Alternative notations ##############################################################################################

The following two alternative "Hello World" implementations are equivalent to the above one, up to an internal representation called Abstract Syntax Tree:

    -- OOPish notation (equivalent to the above, up to Abstract Syntax Tree)
    {{extern "manool.org.18/std/0.5/all"} in Out.WriteLine["Hello, world!"]}

  (note how the first argument now corresponds to a receiver, in OOP parlance),

    -- LISPish notation (ditto)
    {{extern "manool.org.18/std/0.5/all"} in {WriteLine Out "Hello, world!"}}

  (note how `WriteLine`, the target of an applicative expression, is the first element of a Lisp-inspired syntactic list).

<aside markdown="1">
The ability to express essentially the same program (or a part thereof) in different notations may be either extremely useful or completely useless depending on
your situation. In any case, this is allowed not due to a specific design goal but is rather as an artifact of the overall language design.

Some benefits of the "Object-Oriented" notation:
  * You might want to mark the first argument specially, for whatever reason.
  * You might want to use a kind of universal infix notation or pipeline multiple applications while avoiding a lot of nested parentheses, e.g.:
    `X.Map1[].Map2[]`.

Some benefits of the "S-expression" notation:
  * It is already strongly preferred for so-called non-applicative, special expressions.
  * It works better when you want to format some complex applicative expression uniformly.
</aside>

### Code formatting recommendations ####################################################################################

The most basic principles of MANOOL code formatting are depicted in the following example (you are invited to see more principles in action further in this
tutorial):

    -- most recommended formatting for multi-line expressions
    { {extern "manool.org.18/std/0.5/all"} in
      WriteLine[Out; "Hello, world!"]
    }

### Commenting code in MANOOL ##########################################################################################

MANOOL supports two kinds of comments (please see [Comments] for a complete reference):

    {{extern "manool.org.18/std/0.5/all"} in Out.WriteLine[/*Out;*/ "Hello, world!"]} -- this is a comment

[comments]: /specification/core-language/syntax#h:comments "Comments"

---

**Caution!!! Work in progress!!! No more prose below this line!!!**

---

## Assembling Phrases from Fragments
Example:
~~~
-- evaluation of multiple expressions in a row
{ {extern "manool.org.18/std/0.5/all"} in
  Out.Write["Hello"]; Out.Write[", "]; Out.Write["world"]; Out.Write["!"]
  Out.WriteLine[]
  -- multiple arguments to Write/WriteLine
  Out.WriteLine["Hello"; ", "; "world"; "!"]; -- ";"-delimiters are optional
  Out.WriteLine["Hello" ", "; "world" "!"]    -- everywhere where they are allowed
}
~~~
Output:
~~~
Hello, world!
Hello, world!
Hello, world!
~~~
### Concatenation, Infix Operators
Example:
~~~
-- string concatenation
{ {extern "manool.org.18/std/0.5/all"} in
  -- conventional (infix) notation
  Out.WriteLine["Hello" + "," + " " + "world!"]
  -- applicative (prefix) notation (equivalent up to AST)
  Out.WriteLine[(+)[(+)[(+)["Hello"; ", "]; "world"]; "!"]]
  -- LISPish (prefix) notation (ditto)
  Out.WriteLine[{(+) {(+) {(+) "Hello" ", "} "world"} "!"}]
  -- OOPish (infix) notation (ditto)
  Out.WriteLine["Hello".(+)[", "].(+)["world"].(+)["!"]]
}
~~~
Output:
~~~
Hello, world!
Hello, world!
Hello, world!
Hello, world!
~~~

## Simple Arithmetic
Example:
~~~
-- performing some integer arithmetic and displaying results
{ {extern "manool.org.18/std/0.5/all"} in
  -- outputting piece-wise
  Out.WriteLine["2 + 3 = " 2 + 3; ", "; "5 - 2 = " 5 - 2]
  -- explicit conversion to string and concatenation
  Out.WriteLine["2 + 3 = " + Str[2 + 3] + ", " + "5 - 2 = " + Str[5 - 2]]
}
~~~
Output:
~~~
2 + 3 = 5, 5 - 2 = 3
2 + 3 = 5, 5 - 2 = 3
~~~
### Multiplication, Operator Precedence and Associativity
Example:
~~~
{ {extern "manool.org.18/std/0.5/all"} in
  -- precedence (binding strength) and associativity -----
  Out.WriteLine[
  "2 + 3 * 4 - 5 = " 2 + 3 * 4 - 5; ", "; "(2 + (3 * 4)) - 5 = " (2 + (3 * 4)) - 5]
  -- override with explicit grouping -----
  Out.WriteLine["(2 + 3) * (4 - 5) = " (2 + 3) * (4 - 5)]
}
~~~
Output:
~~~
2 + 3 * 4 - 5 = 9, (2 + (3 * 4)) - 5 = 9
(2 + 3) * (4 - 5) = -5
~~~
### Unary Minus
Example:
~~~
{ {extern "manool.org.18/std/0.5/all"} in
  Out.WriteLine["Neg[2] = " Neg[2]; ", "; "5 + ~2 = " 5 + ~2]
}
~~~
Output:
~~~
Neg[2] = -2, 5 + ~2 = 3
~~~
### Division
Example:
~~~
-- integer division and related operations
{ {extern "manool.org.18/std/0.5/all"} in
  -- heading -----
  Out.WriteLine["     8,3   -8,3   8,-3  -8,-3"]
  -- integer division (truncating) -----
  Out.WriteLine[" /    " 8  /  3  "     "  ~8   /  3  "     " 8  /  ~3  "     "  ~8   /  ~3]
  -- remainder from integer division -----
  Out.WriteLine["Rem   " 8.Rem[3] "     " (~8).Rem[3] "     " 8.Rem[~3] "     " (~8).Rem[~3]]
  Out.WriteLine["Rem " 8.Rem[3].Str["2d"] " " (~8).Rem[3].Str["2d"] " " 8.Rem[~3].Str["2d"] " " (~8).Rem[~3].Str["2d"]]
  -- flooring division -----
  Out.WriteLine["Div   " 8.Div[3] "     " (~8).Div[3] "     " 8.Div[~3] "     " (~8).Div[~3]]
  -- modulo (remainder from flooring division) -----
  Out.WriteLine["Mod   " 8.Mod[3] "     " (~8).Mod[3] "     " 8.Mod[~3] "     " (~8).Mod[~3]]
}
~~~
Output:
~~~
     8,3   -8,3   8,-3  -8,-3
 /    2     -2     -2     2
Rem   2     -2     2     -2
Div   2     -3     -3     2
Mod   2     1     -1     -2
~~~

---

Try to figure out what is going on here (answer in Lesson 3):
~~~
{ {extern "manool.org.18/std/0.5/all"} in
  -- what do Write/WriteLine return? -----
  Out.WriteLine[
  Out.Write["Hello"]; " "; Out.Write[", "]; " ";
  Out.Write["world"]; " "; Out.Write["!"]; " "; Out.WriteLine[]]
  -- what are Out, WriteLine, (+)?.. printing "unprintable"? -----
  Out.WriteLine[
  Out; " "; WriteLine; " "; (+); " "; (~); " "; Foo; " "; Bar]
}
~~~
Output:
~~~
Hello, world!
Nil Nil Nil Nil Nil
value/object WriteLine + ~ Foo Bar
~~~


{%endraw%}{%include page_footer.md%}
