---
title:   Emerging Programming Language with a Translator Under 10K Lines of Code
excerpt: CFP 1
---

{%include page_header.md%}{%raw%}


Abstract
----------------------------------------------------------------------------------------------------------------------------------------------------------------

Some well known families of programming languages make particularly *simple* implementations possible: Forth, Lisp, Tcl, Smalltalk, etc., which is good.
I argue that almost none of them have the syntax and/or semantics close enough to those of more conventional languages, such as Python or Java on one hand and C
or Ada on the other hand, which is not so good (since RPN and S-expressions are unconventional, array manipulation in Tcl is tricky, and so on).
The programming language MANOOL is my attempt to address this issue, and the language differs in a number of details from some other less known and *simple*
languages that solve the same problem.

Being a one-person project, MANOOL is designed around one central idea: to maximize the _expressive power_ / _implementation complexity_ ratio.
This implied the goal of providing expressive power of certain (present and past) popular languages, some of which are mentioned above, but I also managed to
develop a translator of MANOOL in less than 10 KLOC in C++11 (yet it exhibits competitive run-time performance compared to, say, the CPython implementation of
Python).

MANOOL is meant to be a *practical* tool rather than a research project (of course prototype language implementations in just a few LOC are broadly known).

You will learn what the design process for such a project looks like, what is the scope of the language in terms of its features, and what are the simple and
straightforward implementation techniques that contribute to its relatively high run-time performance.

About the presenter
----------------------------------------------------------------------------------------------------------------------------------------------------------------

**Alexey Protasov**

Alex is an enthusiastic independent developer with Russian origins (from Saint Petersburg) living in Medellin, Colombia.
He constantly dreams with "better" programming languages and has over 30 years of experience with designing and implementing languages and development tools ---
he had in the past a shareware visual programming tool, worked in the area of compilers at Intel and Sun Microsystems, and taught compiler construction and the
theory of formal languages at a university.

Alex speaks Spanish, English, and Russian.
When he is not working on MANOOL, he likes swimming, traveling, and dancing (Salsa, Porro, Merengue, Cumbia, etc.).

What will the attendee learn? --- Comments for reviewers
----------------------------------------------------------------------------------------------------------------------------------------------------------------

### Overview ###########################################################################################################

MANOOL is a *homoiconic*, *dynamic*, and *multi-paradigm* *general-purpose* programming language with a *functional* core.
Its translator (published under GPLv3) is made for native-code run-time environments, is written in idiomatic C++11 (with a few industry-acknowledged
GCC-specific extensions), and currently runs under several Unix-like operating systems on a number of instruction-set architectures.

MANOOL happens to be a close relative of Lisp-family languages but not by how programs in MANOOL look (its surface syntax is distinct from the syntax of
S-expressions).
Also, unlike Lisp(s) it has what I call a _non-referential data model_ (see below).

### Design goals #######################################################################################################

MANOOL has explicitly stated design goals (ordered according to priority), which are
  1. implementation simplicity (which is the sole most important consideration in the design);
  2. expressive power (in practical sense), usability, and general utility (value for consumers); attention to syntax and semantics details;
  3. correctness, security, and overall quality of implementation; run-time reliability;
  4. run-time performance and scalability; and
  5. consistency, completeness, orthogonality of features and language elegance; conceptual economy and purity.

Note that implementation simplicity is not only important to make the project a viable option for just a single developer but also indirectly contributes to the
*quality* of implementation.

### Syntax #############################################################################################################

Starting right from the syntax, MANOOL is the result of many design trade-offs, and its syntax as such is not ideal (after all, it may still seem to be a bit
surprising to some newcomers).
So, in this talk I'll **explain** why it's important (to achieve the implementation simplicity) for the syntax of MANOOL to remain the way it is (especially
considering some basic constraints such as a limited source character set).

As a crude illustration, a "Hello, world!" program in MANOOL might look like

    {{extern "manool.org.18/std/0.6/all"} in WriteLine[Out; "Hello, world!"]}

  and an expression that evaluates to a recursive factorial function might look like

    { let rec
      { Fact =
        { proc { N } as -- precond: N.IsI48[] & (N >= 0)
        : if N == 0 then 1 else
          N * Fact[N - 1]
        }
      }
      in
      Fact
    }

### Non-referential data model #########################################################################################

The following code fragment is valid in a large number of programming languages (ignoring minor syntactic differences):

    A[1] = 1; B = A; B[1] = 2

  However, there are two possible outcomes after executing it (and hence, two classes of languages):
  * `A[1] == 1` --- non-referential data model used by such languages as C (for `struct`s) and Ada (and MANOOL) and
  * `A[1] == 2` --- referential data model used by such languages as Python and Java.

We'll **talk** about flaws of the purely referential data model, good reasons because of which it has been there in a historical perspective (since Lisp and
later, CLU), alternatives to it adopted in different families of languages, and performance limitations of the simple (and semantically consistent) solution
adopted in MANOOL.

### Features ###########################################################################################################

I'll also at least **touch** all other language feature (apart of those marked with *), which are (according to the official language introduction)
  * Very compact (or even minimalist) core language (up to a point where a meta-circular specification might be appropriate)
  * Convenient standard library (but completely optional to use)
  * Computational primitives based on Church's lambda-calculus (in the spirit of Landin's ISWIM prototype language/ML-like languages)
    - Name bindings with static (lexical) scope
    - Explicit variable capture and classification of name bindings into compile- and run-time
    - Mainly eager (strict) evaluation strategy (from the perspective of lambda-calculus) with possible side effects
  * Compile-time evaluation
  * Metaprogramming:
    - Lisp-like syntactic macros with optional macro hygiene
    - (optionally) self-modifying code
  * Block-structured and expression-oriented (from the perspective of procedural imperative programming)
  * Dynamic (latent) but strong data typing discipline
  * Ad-hoc polymorphism:
    - run-time function/operator overload resolution (via dynamic single-dispatch)
  * Observably (modulo timings) non-referential (by-value) data model encouraging (but not requiring) to use (observably) immutable objects using automatic
    reference counting (ARC) and transparent copy-on-write (COW) implementation techniques (*)
  * Move semantics and syntactic sugar that emulates in-place partial updates (*)
  * Very high-level composite abstract data types (ADTs):
    - set-theoretic operations, comprehensions, and logic quantifications inspired by the math notation and SETL
    - values of any types (as long as the former are totally ordered) can be used as set elements and map keys
    - iterators with elements of lazy evaluation strategy
  * Modular programming:
    - namespaces
    - name binding visibility control
    - multiple source files (plus support for Ada-like private program units)
  * User-defined abstract data types:
    - data encapsulation (with visibility control)
  * Exception handling (with stack unwinding)
  * MANOOL programs can be instructed to recover even from dynamic memory (heap and stack) exhaustion
  * Decimal floating-point arithmetic (out-of-the-box)
  * Multithreading-aware implementation, free from global interpreter lock (GIL)
  * Simple plug-in application programming interface (API)

### Run-time performance ###############################################################################################

And finally we'll **talk** about the simple and relatively straightforward techniques that allowed me to achieve competitive run-time performance (such as VM
operation fusion, which reduces the number of dynamic dispatches, i.e. branches, and careful design of internal VM interfaces and data structures) and
**review** some numbers to compare the implementation performance-wise with other engines.


{%endraw%}{%include page_footer.md%}
