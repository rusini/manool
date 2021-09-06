<img alt="MANOOL Logo" src="https://manool.org/MANOOL-Logo.png" width="64" height="64"> MANOOL v0.6.1
=====================================================================================================

*MANOOL is a fairly readable homoiconic language with primarily value (non-referential) semantics that balances the programmer's productivity with
scalability...*

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
administrator, on multiple occasions I saw serious scalability issues popping up suddenly after a year of production use due to flaws in backend software
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
  * open (homoiconic) approach to language architecture (in the same sense as in Lisp but using a notation alternative to S-expressions and with macro
    definitions applying to their own limited scope), and
  * primarily value (non-referential) semantics with copy-on-write policy under the hood and move operations (and this works even for user-defined abstract data
    types, due to availability of special syntactic sugar).

Both of those principles require things to work slightly differently on the very basic level, which suggests that introducing a whole new language is more
appropriate than trying to extend an existing one.

#### Why should I learn MANOOL?

It depends on who is asking. One possible reason is that playing around with MANOOL means joy and fun that existing mainstream languages can hardly offer to
you. E.g., in brief:

* Assuming `A == {array of 1 2 3}`, after `B = A; A[1] = 0`, `B[1] == 2`. Likewise, after `B = A; B[1] = 0`, `A[1] == 2` -- value semantics.

* On the other hand, `A[1] = 0` (as well as `S = S! + "Hi"`) may have (amortized) O(1) run-time complexity -- thanks to move operations.

* `A[1] = 0` is actually a shorthand for `A = A!.Repl[1; 0]`, and in other contexts `A[1]` is equivalent to `A.Apply[1]` -- unifying syntactic sugar.

* Incidentally, `A.P[...]` just stands for `P[A; ...]` (which could also be written as `(P A; ...)` in a Lisp-ish form) -- more syntactic sugar.

* Polymorphic operations are indistinguishable from regular (first-class) procedures (and at the same time they are just symbols):

      {var {Plus = (+)} in ... 1.Plus[1] ... "Hi".Plus["World"] ... Out.Write[Plus] ...}

* You can construct and index into a key-value mapping with sets as keys. After

      M = {map of {set of 1 2 3} = 1; {set of 4 5 6} = 2}

  `M[{set of 4 5 6}] == 2` -- no arbitrary restrictions on keys or their type, which is partly a consequence of value semantics.

* First-class value bindings involve compile-time evaluation, and similarly you can use handful syntactic sugar to specify constant values, e.g.: `F64["1.1"]$`,
  `D128["1.10"]$`, `Sqrt[F64[2]]$`.

* You can write the whole program unit in some domain-specific language instead of standard MANOOL; just replace `(extern "...")` at program-unit level (see
  complete examples below) with the reference to a different module.

* On the other hand, macro bindings have limited scope (like any other kind of bindings):

      {let {unless = {macro: proc {F} as ...}} in ... {unless ...} ...}

* A module can be introduced at program-unit level by the construct `{let {...} in: export ...}` or, equally, be bound to a name and thus become a local module
  (&agrave; la Modula-2):

      {let {mUtilities = {let {...} in: export ...}} in ... {mUtilities in ...} ...}

* Programs can recover from out-of-memory conditions gracefully and reliably:

      ReserveHeap[...]; {on HeapExhausted do ... after ...}

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

    {{extern "manool.org.18/std/1.0/all"} in Out.WriteLine["Hello, world!"]}

and in the following sample program a recursive factorial function is defined and invoked:

    { {extern "manool.org.18/std/1.0/all"} in
    : let rec { Fact = {proc {N} as:
       if N == 0 then 1 else N * Fact[N - 1]} }
      in
      Out.WriteLine["Factorial of 10 = " Fact[10]]
    }
