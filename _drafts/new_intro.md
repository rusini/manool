---
title:   The Programming Language MANOOL
excerpt: The Programming Language MANOOL
---

{%include page_header.md%}


#### What is the purpose of MANOOL?

MANOOL is a general-purpose language suitable for diverse programming tasks from several problem domains. However, it has substantial bias toward so-called
scripting languages and hopefully represents an evolutionary step over existing scripting languages. Thus, MANOOL should be compared to and has nearly the same
purpose as Python, Ruby, PHP, Scheme, JavaScript, or Lua, which implies properties typical for such languages, e.g.: short edit-compile-test development cycle
(at least for code bases under 1 MLOCs) and run-time type checking.

More than with anything else, MANOOL is compatible with the basic, C/C++, run-time environment and is implemented itself in C++, which allows for easier
integration of programs with many existing libraries and native APIs (which is the case for some scripting languages and not for another). Although MANOOL has a
high-level dynamic semantics, which normally means quite low run-time performance, the goal for the upcoming version of the language and its translator is to
achieve the performance only marginally below that of the most sophisticated, high-end dynamic-language engines in the market (such as V8 and LuaJIT).

#### What real-world problems does your project solve?

MANOOL is intentionally designed in such way that it represents a specific point on a continuum; on one side of this continuum are high-level languages designed
with the programmer's convenience and productivity in mind, and on the other side are languages designed with execution speed and/or solution scalability in
mind (whereas, as a matter of fact, software quality and reliability may be correlated with either of those extremes, depending on the situation). Based on my
past programming experience, I argue that no existing mainstream language addresses both goals in a balanced way. As a result, programs are either more
expensive in development than they should be or do not scale with workload as needed.

Think, for instance, about the number of users and their contributions on a simple
social media Web site. Working as a server infrastructure administrator, on a number of occasions I saw real scalability issues due to backend software
architecture design flaws, including the choice of programming language and/or its implementation.

As a more detailed illustration, using a primarily statically-typed language where composite values like arrays or structures are gratuitously shared between
several points or components in the program in form of mutable objects (which is typical for languages like Java or C#) places an undue cognitive load on
programmers for simple backend "scripting". On the other hand, while execution speed is not always relevant (within certain reasonable constraints), developing
such scripts in typical scripting languages (like Python, Ruby, PHP) turns out to scale poorly with the site workload growth and couting with extra execution
speed sooner or later broadens the domain area of a programming language and/or its implementation.

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

* High contruct orthogonality allowing for more liberal feature combination in creative and even unforeseen ways. For example, modular system...

#### What does it offer to potential project maintainers and contributors?

Since MANOOL is a personal, solo-developer project with severely limited resources, it almost inevitably has to use a straightforward, streamlined, and modular
implementation, which is based on simple algorithms and data-structures (from the compiler theory standpoint). Let's take, for instance, the implementation
size; the MANOOL translator is written in under 10 KLOCs, whereas the most widely used Python interpreter relies on at least 100 KLOCs.

This does not necessarily mean that the MANOOL implementation is cheap or otherwise low-grade but rather that extra development efforts can be committed to
ensuring high implementation quality and reliability. This also implies lower project entry requirements, encouraging more people to contribute to the
development. Besides, such compact code bases are more suitable for educational purposes (than larger ones, which are often full of legacy stuff).

#### Are there any plans for future development?

Although MANOOL has a high-level dynamic semantics, which normally means quite low run-time performance, the goal for the upcoming version of the language and
its translator is to achieve the performance only marginally below that of the most sophisticated, high-end dynamic-language engines in the market (such as V8
and LuaJIT) but only at a fraction of their complexity.

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
