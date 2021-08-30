---
title:   The Programming Language MANOOL
excerpt: The Programming Language MANOOL
---

{%include page_header.md%}


#### What is the purpose of MANOOL?

MANOOL is a general-purpose language suitable for diverse programming tasks from several problem domains. However, it has substantial bias toward scripting and
hopefully represents an evolutionary step over existing scripting languages. Thus, MANOOL should be compared to and can serve the same purpose as Python, Ruby,
PHP, Scheme, JavaScript, or Lua, which implies properties typical for such languages, e.g.: short edit-compile-test development cycle (at least for code bases
under 1 MLOCs) and run-time type checking.

#### What real-world problem does your project solve?

MANOOL is intentionally designed in such way that it represents a specific point on a continuum; on one side of this continuum are high-level languages designed
with the programmer's convenience and productivity in mind, and on the other side are languages designed with execution speed and/or solution scalability in
mind (whereas, as a matter of fact, software quality and reliability may correlate with either of those extremes, depending on the situation). Based on my past
programming experience, I argue that no existing mainstream language addresses both goals at once in a balanced way. As a result, programs are either more
expensive in development than they should be or do not scale with workload as needed.

Think, for instance, about the number of registered users and their contributions on an in-house social-network Web-site. Working as a server infrastructure
administrator, on multiple occasions I saw serious scalability issues to pop up suddenly after a year of production use due to flaws in backend software
architecture, including the choice of programming language and/or its implementation.

As a more elaborate illustration, using a rigid static type system and gratuitous sharing mutable state in the program (widespread in traditional OOP) is far
from how many people think about problem solving in terms of basic (immutable) mathematical objects, and thus this places undue cognitive load on developers for
simple backend scripting. On the other hand, implementing backend algorithms in an average exploratory-programming language (especially in a straightforward
and/or idiomatic way) often leads to poor run-time performance scalability w.r.t. Web-site workload growth.

#### OK, but what warrants a whole new language in case of MANOOL?

Starting off with some relatively unpopular language makes little sense for me as a language designer, since I might miss in this case some improvement
opportunities (whatever it means) while getting almost nothing in return. But why not just extend an existing mainstream language to suite the above stated
goals instead of creating one from scratch?

Achieving competing (and even incompatible) goals is hard and may lead to overly complex and difficult to adopt language designs. MANOOL leverages two
principles in order to deal with this problem, which are currently not observed among mainstream languages:

* open (homoiconic) approach to language architecture (in the same sense as in Lisp but using a notation alternative to S-expressions and with macro definitions
  applying to their own limited scope), and
* primarily value (non-referential) semantics with copy-on-write policy under the hood and move operations (and this works even for user-defined abstract data
  types, due to availability of special syntactic sugar).

This requires things to work slightly differently on the very basic level, which suggests that introducing a whole new language is more appropriate than trying
to extend an existing one.

#### Why should I learn MANOOL?

It depends on who is asking. One possible reason is that playing around with MANOOL means joy and fun that existing mainstream languages can hardly offer to
you. E.g., in brief:

* Assuming `A == (array of 1 2 3)`, after `B = A; A[1] = 0`, `B[1] == 2`. Likewise, after `B = A; B[1] = 0`, `A[1] == 2` (value semantics).

* On the other hand, `A[1] = 0` and `S = S! + "Hi"` may have (amortized) O(1) run-time complexity (thanks to move operations).

* `A[1] = 0` is actually equivalent to `A = A!.Repl[1; 0]`, and in other contexts `A[1]` is equivalent to `A.Apply[1]` (unifying syntactic sugar).

* Incidentally, `A.P[...]` is equivalent to `P[A; ...]`, which could also be written simply as `(P A; ...)` (more syntactic sugar).

* You can construct and index into a key-value mapping with sets as keys. After

      M = (map of (set of 1 2 3) = 1; (set of 4 5 6) = 2)
      
  `M[(set of 4 5 6)] == 2` (no arbitrary restrictions on keys or their type, which is in a sense a consequence of value semantics).

* You can write the whole program unit in some domain-specific language instead of standard MANOOL. Just replace `(extern "...")` (see below) with the reference
  to a module.

* On the other hand, macro bindings have limited scope (like any other kind of bindings):

      (let (unless = (macro: proc (F) as ...)) in ... (unless ...) ...)

* First-class value bindings involve compile-time evaluation, and similarly you can use handful syntactic sugar to specify literal values, e.g.: `F64["1.1"]$`,
  `D128["1.10"]$`.

* A module can be introduced on a program unit level by the construct `(let (...) in: export ...)` or, equally, be bound to a name and thus become a local
  module (&agrave; la Modula-2):
  
      (let (M = (let (...) in: export ...)) in ... (M in ...) ...)

* Polymorphic operations are indistinguishable from normal (first-class) procedures (and at the same time they are just symbols):

      (var (Plus = (+)) in ... 1.Plus[1] ... "Hi".Plus["World"] ... Out.Write[Plus] ...)

* Programs can recover from out-of-memory conditions gracefully and reliably:

      ReserveHeap[...]; (on HeapExhausted do ... after ...)

#### What does it offer to potential project maintainers and contributors?

MANOOL is a personal, solo-developer project with severely limited resources. Thus, to be viable, it almost inevitably has to use a straightforward,
streamlined, and modular implementation, which is based on simple algorithms and data structures (from the compiler theory standpoint). Let's take, for
instance, the implementation size -- the MANOOL translator is written in under 10 KLOCs, whereas the most widespread Python interpreter builds upon at least 100
KLOCs.

This does not necessarily mean that the MANOOL implementation is cheap or otherwise low-grade but rather that extra development effort can be committed to
ensuring high implementation quality and reliability. This also implies lower project entry requirements, encouraging more people to participate in the
development. Besides, such compact code bases are more suitable for educational purposes (than larger ones, which are often full of legacy stuff).

#### Give me a complete example of what code in MANOOL may look like

A "Hello World" program might look like

    ((extern "manool.org.18/std/1.0/all") in Out.WriteLine["Hello, world!"])

(using the second version of the syntax, see below). And in the following sample program a recursive factorial function is defined and invoked:

    ( (extern "manool.org.18/std/1.0/all") in
    : let rec (Fact = (proc (N) as
        : if N == 0 then 1 else N * Fact[N - 1]))
      in
        Out.WriteLine["Factorial of 10 = " Fact[10]])

#### What's next? Do you have a roadmap for MANOOL?

Sure, here it is (*as of September 2021*):

1. Complete a JIT compiler for MANOOL to achieve run-time performance only marginally slower than that of the most sophisticated dynamic-language engines in the
   market (such as V8 and LuaJIT), but only at a fraction of their complexity.

2. Replace `{` and `}` in the syntax by `(` and `)` in the second version of the language. The idea is to to appeal more to at least one established language
   community (Lisp/Scheme) (although at the cost of extra complexity, including a more complicated LL(2) parser).

3. Complete the language specification and the tutorial.

4. Build a MANOOL ecosystem (libraries) and a user community.

---
---
---

#### What's next? Are there any plans for future development?

*[As of September 2021]*

Although MANOOL has a high-level dynamic semantics, which normally means quite low run-time performance, the goal for the upcoming version of the language and
its translator is to achieve the one-fold boost of performance, which is going to be only marginally below that of the most sophisticated, high-end
dynamic-language engines in the market (such as V8 and LuaJIT), but only at a fraction of their complexity.

Another notable change involves using normal parentheses in place of curly braces, mostly in order to appeal more to at least one existing language community
(Lisp/Scheme), although at the cost to using a more complicated LL(2) surface grammar (after usual transformation by left-recursion elimination) and more
complicated (but still reasonable) rules for the programmer. This change is actually reflected in code samples in this wrap-up.


{%include page_footer.md%}
