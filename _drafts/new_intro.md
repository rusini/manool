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

Achieving competing and even contradictory goals is hard and may lead to overly complex and difficult to adopt language designs. MANOOL leverages two principles
in order to deal with this problem, which are currently not observed among mainstream languages:

* open (homoiconic) approach to language architecture (in the same sense as in Lisp but using an alternative to S-expressions and with syntactic macro
  definitions applying to their own limited scope in the program), and
* primarily value (non-referential) semantics with copy-on-write policy under the hood and move operations (and this works even for user-defined abstract data
  types).

These mechanisms require things to work slightly differently on the very basic level, which suggests that introducing a whole new language is more appropriate
than trying to extend an existing one.

#### Why should I learn MANOOL?

It depends on who is asking. One possible reason is that programming in MANOOL means joy and fun that existing mainstream languages can hardly afford.

In brief:

* Assuming `A == (array of 1 2 3)`, after `B = A; A[1] = 0`, `B[1] == 2`. Likewise, after `B = A; B[1] = 0`, `A[1] == 2` (value semantics, see above).

* On the other hand, `A[1] = 0` or `S = S! + "Hi"` may have (amortized) O(1) run-time complexity (thanks to move operations).

* `A[1] = 0` is actually equivalent to `A = A!.Repl[1; 0]`, and in other contexts `A[1]` is equivalent to `A.Apply[1]` (unifying syntactic sugar).

* You can construct and index into a key-value mapping with sets as keys. After

      M = (map of (set of 1 2 3) = 1; (set of 4 5 6) = 2)
      
  `M[(set of 4 5 6)] == 2` (no arbitrary restrictions on keys or their type).

* Use can write the whole program unit in your very own domain-specific language instead of standard MANOOL, just replace `((extern ...) in ...)` (see below)
  with a reference to your own module.

* On the other hand, macro bindings have limited scope (like any kind of bindings):

      (let (unless = (macro: proc (F) as ...)) in ... (unless ...) ...)

* Modules are introduced by a construct like `(let (...) in: export ...)`, which can be used on a program unit level or bound to a name and become a local
  module (as in Modula-2). Incidentally, a module can be imported into a limited scope: `(M in ...)`.

* "Overloaded" operators and "methods" behave normal (first-class) procedures (and at the same time they are just symbols):

      (let (Plus = (+)) in ... 1.Plus[1] ... "Hi".Plus["World"] ... Out.Write[Plus] ...)

#### Why should we, practicing software engineers, learn your language?

Like any other language, MANOOL provides a unique blend of features, some of which may make sense for you and some may be irrelevant. I do not intend to list
all of them here. Instead, I suggest to pick one or more of the most uncommon and distinctive approaches and paradigms (in respect of existing mainstream
languages) that you should experiment with and maybe make a workhorse for your next project:

* Extensible (and homoiconic) architecture in the spirit of Lisp-family languages but without most of peculiarities and practical difficulties of
  classic S-expressions (actually no blame is intended here)
   
  An extremely small language core with very few built-in features allows for higher extensibility and definition of many useful DSLs on top of core MANOOL.

      ( (extern "manool.org.18/std/1.0/all") in
      : let ( do = (macro
          : proc (F) as
          : if (Size[F] == 4) & (F[2] == unless') then
              (array of if# (array of (~)# F[3]) then' F[1])
            else
              (array of do#) + F[Range[1; Size[F]]]) )
        in
          (do Out.WriteLine["Success"] unless False)
          (do Out.WriteLine["Failure"] unless True) )



* As it is the case for languages like SETL or Python, a large number of high-level composite data types, like sets, mappings (dictionaries), arrays, and
  sequences provides a handful and reasonable (performance-wise) choice of built-in data structures to deal with composite data. Example: ...

* Value semantics (as opposed to reference semantics) is used by default, with the implementation based on the form of lazy copying (copy-on-write) techniques,
  which also implies more deterministic object lifecycle management compared to tracing GC (whereas the problem of shared-memory concurrent reference counting
  is taken out of the equation). Value semantics makes the language much more on the functional programming side, but without disallowing assignments entirely
  (as such, idiomatic MANOOL is already based on typical functional constructs, like first-class procedures and block-scope lexical value bindings). Example:
  ...

* The language specification, which stipulates precisely run time and space requirements of your programs and allows for easy reasoning about and control over
  those requirements. For instance, MANOOL supports move semantics when manipulating large composite values (which is reminiscent of linear or uniqueness
  typing)

* Full support for thread-based concurrent and parallel programming (without using a parallelization-inhibiting global interpreter lock, typical for Python,
  Ruby, and JavaScript runtimes)

* Reasonably high run-time performance

  According to my own benchmarking, we could range languages and/or their implementation according to execution speed in the following way: 1. C/C++, 2. Java,
  C#, and similar, 3. Julia, JavaScript and Lua (via LuaJIT), 4. some high-speed implementations of Scheme and Common Lisp, 5. most typical scripting
  languages, like Python, PHP, and Perl, and 6. some of the slowest scripting languages and/or their implementaions, like Tcl or Bash. On a number of
  benchmarks MANOOL can be placed slightly below (3) and slightly below (4), typically exposing the one fold program execution speedup over most typical
  scripting languages and (only) 15 times slowdown over fastest equivalent programs written in C or C++. This is because inspite of being a high-level
  scripting language and having strict JIT compilation run-time constraints, MANOOL nevertheless employs simple and efficient native-code compilation
  techniques.

* Enhanced program reliability. You can, for instance, easily integrate in your program a recovery strategy even after exhaustion of dynamically allocated
  memory (heap).

* High construct orthogonality allowing for more liberal feature combination in creative and even unforeseen ways. For example, modular system...



MANOOL can give you a sense of joy Python programming although with no arbitrary limitations, and at the same time gives you control over resources (objects)
that represent idealized mathematical values right when and where you need it.

* As it is the case for languages like SETL or Python, a large number of high-level composite data types, like sets, mappings (dictionaries), arrays, and
  sequences provides a handful and reasonable (performance-wise) choice of built-in data structures to deal with composite data. Example: ...

The principle of MANOOL is to allow the programmer to provide specific hints about what is needed where this is easy and automate tasks where its is harder and
thus more appropriate. For instance, the programmer is to choose an internal data structure for composite data instead of using more generic or universal
composite data (like "objects" in JavaScript or "tables" in Lua). At the same time, the MANOOL runtime decides rather transparently when objects are cloned
(based on reference counting).

#### What does it offer to potential project maintainers and contributors?

MANOOL is a personal, solo-developer project with severely limited resources. Thus, to be viable, it almost inevitably has to use a straightforward,
streamlined, and modular implementation, which is based on simple algorithms and data structures (from the compiler theory standpoint). Let's take, for
instance, the implementation size -- the MANOOL translator is written in under 10 KLOCs, whereas the most widely used Python interpreter relies on at least 100
KLOCs.

This does not necessarily mean that the MANOOL implementation is cheap or otherwise low-grade but rather that extra development efforts can be committed to
ensuring high implementation quality and reliability. This also implies lower project entry requirements, encouraging more people to participate in the
development. Besides, such compact code bases are more suitable for educational purposes (than larger ones, which are often full of legacy stuff).

#### Are there any plans for future development?

Although MANOOL has a high-level dynamic semantics, which normally means quite low run-time performance, the goal for the upcoming version of the language and
its translator is to achieve the one-fold boost of performance, which is going to be only marginally below that of the most sophisticated, high-end
dynamic-language engines in the market (such as V8 and LuaJIT), but only at a fraction of their complexity.

Another notable change involves using normal parentheses in place of curly braces, mostly in order to appeal more to at least one existing language community
(Lisp/Scheme), although at the cost to using a more complicated LL(2) surface grammar (after usual transformation by left-recursion elimination) and more
complicated (but still reasonable) rules for the programmer. This change is actually reflected in code samples in this wrap-up.

#### Give me a quick example of what code in MANOOL looks like

A "Hello World" program might look like (in the second version of the language)

    ((extern "manool.org.18/std/1.0/all") in Out.WriteLine["Hello, world!"])

And in the following sample program a recursive factorial function is defined and used:

    ( (extern "manool.org.18/std/1.0/all") in
    : let rec (Fact = (proc (N) as
        : if N == 0 then 1 else N * Fact[N - 1]))
      in
        Out.WriteLine["Factorial of 10 = " Fact[10]])


---
---
---

achieves the above goals while being based on an open
(homoiconic) approach to language design. Also, to achieve those goals, MANOOL introduces a non-so-prevalent data model, called non-referential semantics, and
supports so-called move operations, which suggests that introducing a new language is simpler than trying to extend an existing one.

Achieving contradictory goals is hard and may lead to complex and difficult to adopt designs. MANOOL achieves the above goals while being based on an open
(homoiconic) approach to language design. Also, to achieve those goals, MANOOL introduces a non-so-prevalent data model, called non-referential semantics, and
supports so-called move operations, which suggests that introducing a new language is simpler than trying to extend an existing one.




More than with anything else, MANOOL is compatible with the basic, C/C++, run-time environment and is implemented itself in C++, which allows for easier
integration of programs with many existing libraries and native APIs (which is the case for some scripting languages and not for another). Although MANOOL has a
high-level dynamic semantics, which normally means quite low run-time performance, the goal for the upcoming version of the language and its translator is to
achieve the performance only marginally below that of the most sophisticated, high-end dynamic-language engines in the market (such as V8 and LuaJIT).






In addition to the above, MANOOL provides a combination of the following features. Pick one or more of them that apply in your case:



More that with anythig else, MANOOL is compatible with the basic, C/C++, run-time environment and is implemented itself in C++, which allows for easier
integration of programs with many existing libraries and native APIs. Although MANOOL has a high-level dynamic semantics, which typically means quite slow program
execution, the goal for the upcoming version of MANOOL and its translator is to achieve run-time performance close to the performance of high-end engines for
such languages (e.g., V8 and LuaJIT).



MANOOL is a general-purpose language suitable for diverse programming tasks from several problem domains. However, it has substantial bias toward so-called
scripting languages and, hopefully, represents an evolutionary step over existing scripting languages. Thus, MANOOL can be compared to and has nearly the same
purpose as, for example, Python, Ruby, PHP, or Scheme, which implies properties typical for such languages, e.g.: short edit-compile-test development cycle
(even for multimillion LOC code bases, in case of MANOOL) and run-time data-type checking.

MANOOL is to be most compatible with the basic run-time environment for modern OSes (that is, the C/C++ runtime), which allows for easier integration of the
language with many existing libraries and native APIs. In spite of being a high-level dynamic language, where typically means quite slow program execution, the
goal for the upcoming version of MANOOL and its translator is to achieve run-time performance close to the performance of high-end engines for such languages
(e.g., V8 and LuaJIT).






MANOOL is a general-purpose programming language suitable for a range of diverse programming tasks in several problem domains but
has a moderate bias toward so-called scripting languages and, hopefully, represents an evolutionary step
over existing scripting languages.
Thus, it can be compared to and has nearly the same purpose as, for example, Python, Ruby, PHP, or Scheme
programming language, which implies properties typical for many scripting languages, like short
edit-compile-test development cycle (even for multimillion LOC code bases) and run-time type checking.
MANOOL run-time environment is most compatible with native C/C++ environment allowing for easier
integration with many existing libraries and APIs for native platforms.


What real-world problems does your project solve?
-------------------------------------------------

MANOOL is intentionally designed in such way that it represents a specific point on a continuum where on one side are high-level languages designed with
programmer's convenience and productivity in mind, and on the other side are languages designed with execution speed and/or solution scalability in mind
(whereas software quality and reliability may be associated with either of those extremes, depending on the situation). I argue that no existing mainstream
language addresses both goals in a balanced way (a conclusion based on my past programming experience). As a result, programs are either more expensive in
development or do not scale with workload as they could be. Think, for instance, about the number of users and their contributions on a simple
social media Web site. Working as a server infrastructure administrator, on a number of occasions I saw real scalability issues due to backend software
architecture design flaws, including the choice of programming language and/or its implementation.

As a more detailed illustration, using a primarily statically-typed language where composite values like arrays or structures are gratuitously shared between
several points or components in the program in form of mutable objects (which is typical for languages like Java or C#) places an undue cognitive load on
programmers for simple backend "scripting". On the other hand, while execution speed is not always relevant (within certain reasonable constraints), developing
such scripts in typical scripting languages (like Python, Ruby, PHP) turns out to scale poorly with the site workload growth and couting with extra execution
speed sooner or later broadens the domain area of a programming language and/or its implementation.




{%include page_footer.md%}
