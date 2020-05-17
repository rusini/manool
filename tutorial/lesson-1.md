---
title:   Lesson 1 -- Tutorial
updated: 2020-05-16
---

Go to Lesson:&ensp;[1]&ensp;\|&ensp;[2]&ensp;\|&ensp;[3]&ensp;\|&ensp;[4]&ensp;\|&ensp;[5]

[1]: #start
[2]: lesson-2#start
[3]: lesson-3#start
[4]: lesson-4#start
[5]: lesson-5#start

<aside markdown="1" class="right">
After completing this tutorial, you will be able to
  * develop MANOOL programs of moderate complexity,
  * use a variety of MANOOL composite data types to deal conveniently with structured data, and
  * freely navigate through the MANOOL specification (which is more formal and harder to read).
</aside>

{%include page_header.md%}{%raw%}


Hello World
----------------------------------------------------------------------------------------------------------------------------------------------------------------

The following is a traditional "Hello World" program in MANOOL:

    -- Hello World program -- in "applicative" notation
    {{extern "manool.org.18/std/0.5/all"} in WriteLine[Out; "Hello, world!"]}

You can play with MANOOL examples by using the [online evaluator] or running them from the command line according to the [instructions]. Here are just a couple
of ideas about how to run MANOOL programs from the command line:

    mnlexec hello.mnl

  (assuming you have placed your source code into `hello.mnl`) or, for short scripts:

    mnlexec <(echo $'{{extern "manool.org.18/std/0.5/all"} in WriteLine[Out; "Hello, world!"]}')

  (we use `$'...'` with a leading `$` here just to be sure we can more easily escape `'` characters in Bash in the future).

The expected output is unsurprisingly

    Hello, world!

Using the online evaluator is convenient but has certain limitations. Whatever approach you choose, first consult [How to Download and Install
MANOOL][instructions] for more tips.

[online evaluator]: /eval     "MANOOL Online Evaluator"
[instructions]:     /download "How to Download and Install MANOOL"

#### How it works

1. The first line `-- ...` is just a comment, which is ignored by the MANOOL translator (see later).

2. The construct `WriteLine[Out; "..."]` on the second line resembles a *function call* in many languages, where the construct `WriteLine` would specify a
   function, and `Out` and `"Hello, world!"` would be the arguments.[^a1] Here, as a side effect of an evaluation of this expression, the phrase `Hello,
   world!`, specified by a string literal, eventually appears on the standard output, specified by the argument `Out`.

3. During compilation of the whole expression `{{extern "..."} in ...}`, all identifier definitions (particularly the one for `Out`)[^a2] from the standard
   library module specified by the string literal `"manool.org.18/..."` are *imported* into the scope that follows the keyword `in`.[^a3]

[^a1]: In MANOOL ordinary functions are called _procedures_, and the construct `WriteLine[Out; "..."]` is more generally called an _applicative expression_
       (more on this later). In overall, MANOOL is a multiparadigm language with a functional core.

[^a2]: ...or rather their analogs in MANOOL called _bindings_...

[^a3]: `{extern "..."}` actually has its own meaning here (more on this later), but for now you can safely ignore this fact.

<aside markdown="1">
This has nothing special by now and is quite a norm for a lot of languages nowadays, and I promise all of this is going to become more interesting later.
However, you may note that the main difference is in the (surface) syntax and the unusual choice of brackets and delimiters. Please bear in mind for now that
this is not an arbitrary choice nor a matter of personal preference but rather a result of many design trade-offs in the language.
</aside>

### Alternative notations ##############################################################################################

The following two alternative "Hello World" implementations are equivalent to the above one, up to an internal representation called Abstract Syntax Tree:

    -- Hello World program -- OOP-ish notation (equivalent to the above, up to Abstract Syntax Tree)
    {{extern "manool.org.18/std/0.5/all"} in Out.WriteLine["Hello, world!"]}

  (note how the first argument now corresponds to a receiver, in OOP parlance),

    -- Hello World program -- LISP-ish notation (ditto)
    {{extern "manool.org.18/std/0.5/all"} in {WriteLine Out "Hello, world!"}}

  (note how `WriteLine`, the _target_ of an applicative expression, is now the first element of a Lisp-inspired syntactic list)[^a4].

[^a4]: The later example uses the notation that reflects Abstract Syntax Trees (ASTs) for all syntactic constructs in MANOOL in the most straightforward way
       (like S-expressions in Lisps).

<aside markdown="1">
The ability to express essentially the same program (or a part thereof) in different notations may be either extremely useful or completely useless depending on
your situation. In any case, this is allowed not due to a specific design goal but rather as an artifact of the overall language design, which features a simple
and uniform syntax called a *universal* syntax.

Some benefits of the "Object-Oriented" notation:
  * You might want to mark the first argument specially, for whatever reason.
  * You might want to use a kind of universal infix notation or pipeline multiple operations while avoiding a lot of nested parentheses, e.g.:
    `X.Map1[].Map2[].Map3[]`.

Some benefits of the "S-expression" notation:
  * It is already strongly preferred for so-called _special_ (non-applicative) expressions.
  * It works better when you want to format some complex applicative expression uniformly, which is demonstrated at the end of this lesson.
</aside>

### Code formatting recommendations ####################################################################################

MANOOL is a free-form and case-sensitive language. The most basic principles of MANOOL code formatting are illustrated in the following example (you are invited
to see for yourself more principles in action further in this tutorial):[^a5]

    -- Most recommended formatting for multi-line expressions
    { {extern "manool.org.18/std/0.5/all"} in
      WriteLine[Out; "Hello, world!"]
    }

[^a5]: Nonetheless, this particular example might fit better on a single line as we've already seen.

### Commenting code in MANOOL ##########################################################################################

MANOOL supports two kinds of comments (please see [Comments] for a complete reference):

    {{extern "manool.org.18/std/0.5/all"} in Out.WriteLine[/*Out;*/ "Hello, world!"]} -- this is a comment

[comments]: /specification/core-language/syntax#h:comments "Comments"


Combining Multiple Expressions
----------------------------------------------------------------------------------------------------------------------------------------------------------------

Now let's assemble the `Hello, world!` phrase from several fragments. Although this may sound a bit boring, this task is perfect to illustrate how to combine
multiple applicative expressions together.

First, to stipulate their sequential evaluation (one after another), just for the purposes of side effects (i.e., ignoring any results they return), you can
simply write down the expressions one after another, optionally delimiting them by semicolons (`;`):[^b1]

    { {extern "manool.org.18/std/0.5/all"} in -- Evaluation of multiple expressions in a row
      Out.Write["Hello"]; Out.Write[", "]; Out.Write["world"]; Out.Write["!"]
      Out.WriteLine[]
    }

  (you can replace `WriteLine` with just `Write` to avoid producing newlines).

[^b1]: We are switching here to the "Object-Oriented" notation and multi-line formatting just to dive into more real-world code examples as soon as possible.

Output:

    Hello, world!

#### Using multiple arguments

You can instead specify more than two arguments to `WriteLine` (or `Write`) to achieve the same net effect:

    { {extern "manool.org.18/std/0.5/all"} in -- Several arguments to WriteLine
      Out.WriteLine["Hello"; ", "; "world"; "!"]; -- semicolon delimiters are optional
      Out.WriteLine["Hello" ", "; "world" "!"]    -- everywhere where they are allowed at all
    }

  (note how the rule regarding optionality of semicolons applies here as well)[^b2].

[^b2]: Similar to the case of multiple notations, this feature is rather unintentional but may be useful in some cases.

Output:

    Hello, world!
    Hello, world!

### String concatenation, infix operators ##############################################################################

Or, you can apply the conventional infix operator `+` to several string values to produce a concatenation of them and output the result as usual:

    { {extern "manool.org.18/std/0.5/all"} in -- String concatenation
      Out.WriteLine["Hello" + ", " + "world" + "!"] -- (true) infix notation
    }

Output:

    Hello, world!

An infix/prefix/postfix operator in MANOOL is actually nothing more than a normal _symbol_ (like `Write`/`WriteLine`) specially recognized and "desugared" by
the MANOOL parser. To suppress such special treatment, enclose an operator in parentheses (as in `(+)`).

To illustrate how alternative notations would work and what the properly infix notation actually maps to (preserving the original AST):[^b3]

    { {extern "manool.org.18/std/0.5/all"} in -- String concatenation -- alternative notations
      Out.WriteLine[(+)[(+)[(+)["Hello"; ", "]; "world"]; "!"]] -- applicative (prefix) notation (equivalent up to AST)
      Out.WriteLine[{(+) {(+) {(+) "Hello" ", "} "world"} "!"}] -- LISPish (prefix) notation (ditto)
      Out.WriteLine["Hello".(+)[", "].(+)["world"].(+)["!"]]    -- OOPish (infix) notation (ditto)
    }

  (note that the `+` operator is, unsurprisingly, left-associative)[^b4].

[^b3]: The following alternative constructs actually make no sense otherwise (in most practical cases).

[^b4]: Nonetheless, this does not affect a result of concatenation.

Output:

    Hello, world!
    Hello, world!
    Hello, world!

Simple Arithmetic
----------------------------------------------------------------------------------------------------------------------------------------------------------------

Let's perform some simple arithmetic operations on integral numbers.

To display results, you can directly pass the values to `WriteLine` (or `Write`), or you can first explicitly convert the values to a string representation by
applying the `Str` operation:[^b5]

    { {extern "manool.org.18/std/0.5/all"} in -- Performing Integer arithmetic operations and displaying results
      Out.WriteLine["2 + 3 = " 2 + 3; ", "; "5 - 2 = " 5 - 2]                 -- outputting piece-wise
      Out.WriteLine["2 + 3 = " + Str[2 + 3] + ", " + "5 - 2 = " + Str[5 - 2]] -- explicit conversion to String and concatenation
    }

[^b5]: MANOOL is a strongly typed language where `5` is not the same thing as `"5"`, and implicit coercion has actually limited scope.

Output:

    2 + 3 = 5, 5 - 2 = 3
    2 + 3 = 5, 5 - 2 = 3

#### Multiplication, operator precedence and associativity

You can also use more complex expressions involving multiple operators, which follow nearly conventional precedence and associativity rules:

    { {extern "manool.org.18/std/0.5/all"} in -- Demonstrating operator precedence (binding strength) and associativity
      Out.WriteLine["2 + 3 * 4 - 5 = " 2 + 3 * 4 - 5; ", "; "(2 + (3 * 4)) - 5 = " (2 + (3 * 4)) - 5]
      Out.WriteLine["(2 + 3) * (4 - 5) = " (2 + 3) * (4 - 5)] -- overriding with explicit grouping of operands
    }

All these operators associate to the left, and `*` binds tighter than `+` and `-` unless where explicitly overriden with parentheses (please refer to [Syntactic
Analysis] for the complete formal grammar of MANOOL).[^b6]

[^b6]: MANOOL has simple operator precedence rules inspired in classic (Pascal-family) languages. In particular, unlike Lisp, Smalltalk, and APL, MANOOL does
       have precedence rules, but unlike ML and Haskell it has a limited set of operators that occupy few fixed precedence levels. MANOOL uses only 4 levels for
       infix operators.

Output:

    2 + 3 * 4 - 5 = 9, (2 + (3 * 4)) - 5 = 9
    (2 + 3) * (4 - 5) = -5

[syntactic analysis]: /specification/core-language/syntax#h:syntactic-analysis "Syntactic Analysis"

#### Unary minus

In MANOOL an arithmetic negation (or unary minus) operation on a number can be always performed by applying the symbol `Neg` to the argument. Alternatively, the
prefix operator `~` works the same way for most (but not all) types of numbers.[^b7] Here is a short example:

    {{extern "manool.org.18/std/0.5/all"} in Out.WriteLine["Neg[2] = " Neg[2]; ", "; "5 + ~2 = " 5 + ~2]}

[^b7]: One disadvantage of the `~` operator is that it is also overloaded to mean Boolean/bitwise negation/complement for some data types, and one advantage is
       that it resembles more the traditional notation `-` (which cannot be used as-is in MANOOL because of grammar ambiguity introduction in such case).

Output:

    Neg[2] = -2, 5 + ~2 = 3

### Division ###########################################################################################################

From a number-theoretic as well as practical standpoint, MANOOL provides a fairly comprehensive set of division-related operations on integral numbers:[^b8]
`(/)`, `Rem`, `Div`, and `Mod`.

[^b8]: This is partly because MANOOL and its specification strive to be extremely accurate about the exact semantics and behavior of various operations in the
       language.

Without going into much detail about Integer division, which is beyond the scope of this tutorial, let's construct a simple program that displays on the
standard output a table that illustrates how these operations work (please see [Integer operations] for a complete reference):

    { {extern "manool.org.18/std/0.5/all"} in -- Integer division and related operations
      -- Heading
      Out.WriteLine["     +8,+3 -8,+3 +8,-3 -8,-3"]
      -- Integer division (truncating)
      Out.WriteLine[" / " (8  /  3).Str["+6d"] (~8   /  3).Str["+6d"] (8  /  ~3).Str["+6d"] (~8   /  ~3).Str["+6d"]]
      -- Remainder from Integer division
      Out.WriteLine["Rem"  8.Rem[3].Str["+6d"] (~8).Rem[3].Str["+6d"]  8.Rem[~3].Str["+6d"] (~8).Rem[~3].Str["+6d"]]
      -- Flooring division
      Out.WriteLine["Div"  8.Div[3].Str["+6d"] (~8).Div[3].Str["+6d"]  8.Div[~3].Str["+6d"] (~8).Div[~3].Str["+6d"]]
      -- Modulo (remainder from flooring division)
      Out.WriteLine["Mod"  8.Mod[3].Str["+6d"] (~8).Mod[3].Str["+6d"]  8.Mod[~3].Str["+6d"] (~8).Mod[~3].Str["+6d"]]
    }

For certain simple data types, the `Str` operation can accept an optional string argument that is a C `printf` conversion specification (excluding an initial
`%`).

Output:

         +8,+3 -8,+3 +8,-3 -8,-3
     /     +2    -2    -2    +2
    Rem    +2    -2    +2    -2
    Div    +2    -3    -3    +2
    Mod    +2    +1    -1    -2

[integer operations]: /specification/standard-library/basic-data-types#h:polymorphic-operations "Integer operations"


Quiz
----------------------------------------------------------------------------------------------------------------------------------------------------------------

Try to figure out what is going on here (you should have acquired all the clues after completing the [Lesson 3](Lesson-3#start)):

    { {extern "manool.org.18/std/0.5/all"} in
      { (WriteLine) Out -- What do Write/WriteLine return?
        Out.Write["Hello"] ", " Out.Write[", "] ", " Out.Write["world"] ", " Out.Write["!"] ", "
        Out.WriteLine[]
      }
      { (WriteLine) Out -- What are Out, WriteLine, (+)?.. Or can we print "unprintable" stuff?
        Out ", " WriteLine ", " (+) ", " (~) ", " Foo ", " Bar
      }
    }

Output:

    Hello, world!
    Nil, Nil, Nil, Nil, Nil
    value/object, WriteLine, +, ~, Foo, Bar


<aside markdown="1" class="right">
Lesson 1 of 5 -- Tutorial
=========================
+ **[Next Lesson](lesson-2#start)**
+ [Lesson 3](lesson-3#start)
+ [Lesson 4](lesson-4#start)
+ [Lesson 5](lesson-5#start)
</aside>

{%endraw%}{%include page_footer.md%}
