---
title:   Native Run-Time Performance for a High-Level, Dynamically Typed Programming Language
updated: 2020-10-04
excerpt: >-
    MANOOL evolves into a general-purpose language equally suitable for exploratory programming and systems programming, and even high-performance computing
---
{%include post_header.md%}{%raw%}


Since popularization of my project MANOOL has been apparently going to nowhere, I've decided to rather invest some time in my self-education, learning new
technologies and pondering about new opportunities. Surprisingly this has resulted in a concrete and viable plan for the future improvements (that is, for
MANOOL-2). If someone would like to join the project at this stage, I would be glad (any help is welcome, even with just testing the concepts).

For those who are not familiar with the project: MANOOL seeks to bridge the gap, in the least troublesome way, between the exploratory style of programming (for
which languages like PHP, Python, Ruby, JavaScript, or Scheme are normally used) and the more thorough style (where languages like C, C++, Java, or Rust are a
better fit), and this is something I was in fact thinking about for more than 30 years.

So, according to the plan, MANOOL evolves into a general-purpose language equally suitable for exploratory programming and systems programming, and even
high-performance computing (at least on traditional computer architectures, since nowadays some HPC solutions run on GPGPU and FPGA devices, and hypothetically
on emerging quantum computers, each case demanding a particular coding style usually available only in specialized, domain-specific programming languages).

For certain reasons (not discussed here due to lack of space) exploratory programming normally involves high-level semantics and especially the dynamic typing
discipline (when data types in programs are associated with values or objects at run time instead of variables or expressions during program compilation, as
opposed to the static typing). On the other hand, systems programming and HPC presume, well, high run-time performance. These properties conflict with each
other, since dynamic typing usually means that computers make more decisions at program run time, which slows down performance by itself and also hinders
further performance optimizations.

Nonetheless, real-world applications often consist of components with different flexibility and performance requirements. For instance, an application may
include inherently dynamic event-driven user interface code and much more static domain area (back-end) code where most hot (critical) instruction paths are
concentrated.

Sophisticated (and expensive in implementation) JIT compilation techniques (used, e.g., in V8 and Mozilla's JavaScript VMs and LuaJIT), including the so-called
tracing JIT, allow you to gain great performance for dynamic languages. Still, such techniques hardly satisfy the above goal and offer notably lower performance
than classic ahead-of-time compilation for equivalent programs written in an inherently static language (such as C, Modula-2, Ada, or Rust, to name a more
recent language); the slowdown may be somewhere between 4 and 10 times (which is still an impressive improvement compared to what more affordable
implementations offer). This happens because in practice such VMs have to anticipate the program execution profile (and hence data types) at run time (with
varying success) instead of exploiting static hints the programmer might provide about the profile, either explicitly or rather implicitly.

Due to the conflict described above, other languages that do achieve the above goal (e.g., Objective-C) are normally hybrid languages that solve the problem by
combining and providing both low-level but high-performance features with high-level but low-performance ones (for instance, Objective-C semantically and even
syntactically looks like a mix between C and Smalltalk).

The approach MANOOL-2 adopts is different: MANOOL-2 is essentially a dynamically typed language with no explicit HPC-related features (such as static types),
but its type system is specifically devised to enable significant amount of type inference during compilation (with sporadic or rather implicit help from the
programmer). In MANOOL-2 this inference is based on long-established data and control flow analysis algorithms and function inlining, and there seems to be an
intimate connection between type inference and constant/value/condition propagation (including their interprocedural variants). Note that typing discipline
(static vs dynamic) is orthogonal to this issue: there is still no such thing as "false negatives due to failed type checks" in MANOOL-2.

The advantage of this approach is that the programmer uses a more compact language and thus has to master fewer features and make fewer decisions as to which
features to use in each particular case and for each particular component of the program (the programmer still should be aware of how the compiler infers types
and performs other deductions and which coding techniques lead to the maximum performance boost in hot paths, but performance hints can be introduces gradually,
if needed at all).

Perhaps the closest such project is Julia. However, Julia is specifically oriented on the area of scientific computing, has high startup times, and still offers
suboptimal performance (albeit better than JavaScript or Lua). MANOOL-2 should overcome such issues, and it is a viable goal according to my preliminary
experiments.

Note that apart from higher run-time performance, statically typed languages are also traditionally associated with higher software engineering standards, as
opposed to "quick-and-dirty" exploratory style solutions. However, the position MANOOL-2 adopts is that a sophisticated static type system used for defect
preventing purposes (while being useful in practice) should belong better to external tools and not to the programming language itself (though, the type system
of MANOOL-2 makes it more suitable for programming in-the-large in comparison to an ordinary dynamically typed language).

All of the above is not just a business idea. I have actually performed some experiments and studied viability of the optimization algorithms and (what's most
important) what limitations of such algorithms can and should be condoned in practice. And of course, there is also the current version of MANOOL as a starting
point. In conclusion and as a matter of simple illustration, here is a piece of code in MANOOL-2 with some comments regarding its expected high-performance
hallmarks:

    { {extern "manool.org.18/std/2.0/all"} in
    : let
      Fold = -- left-fold some elements yielded by some generator G
      { proc I; Op; G as inline -- polymorphic procedure
      : do I after   -- refcounting for G on entry/exit is optimized out
      : for E = G do -- iterate over elements in RAM, no dynamic dispatch
        I = I!.Op[E] -- just "addsd" on x86, no dispatch or type checks
      }
      in
    : let
      Avg = -- average elements of an array A of Binary64 floats
      { proc A as -- monomorphic procedure
        {assert Size[A.as[{array F64}]] > 0} -- tiny O(1) overhead
        Fold[F64[0]$; (+); A] / F64[Size[A]] -- no dispatch or type checks
      }
      in
      -- The return type of Avg is actually known at compile-time - F64:
      Out.Write_line[Avg[{array F64}[F64[1] F64[2] F64[3] F64[4] F64[5]]$]]
      Out.Write_line[Avg[{array I64}[1 2 3 4 5]$]] -- signals Type_mismatch
    }


{%endraw%}{%include post_footer.md%}
