---
title:   Native Run-time Performance for Dynamic Programming Languages
excerpt: CFP 2
---

{%include page_header.md%}{%raw%}


Abstract
----------------------------------------------------------------------------------------------------------------------------------------------------------------

Much has been told about advantages and disadvantages of static vs dynamic typing, without reaching a definitive conclusion. Being a low-budget project, the
programming language MANOOL has to seek major conceptual economy, leading to a compact implementation. For instance, MANOOL is a homoiconic language (like Lisp
or Tcl). On the other hand, its design and implementation simplicity is the most compelling reason for the dynamic typing choice.

Static typing is often associated with languages that admit high-performance implementations (e.g., C or Java), but this does not always have to be the case.
For instance, sophisticated dynamic specialization (e.g., in case of JavaScript/V8 or LuaJIT) bridges the gap between static and dynamic typing languages.
However, such techniques may be prohibitively expensive for a small project like MANOOL, while being still insufficient to reach the highest possible run-time
performance (achievable for C/C++).

MANOOL is specifically designed to admit effective static optimizations. While not a new idea (e.g., Julia has the necessary properties for that), it seems that
it has not been duly explored in mainstream programming yet (and hopefully, MANOOL has a more general-purpose design than Julia).

You will learn about the overall architecture of MANOOL and its compiler, the intermediate representations, the sources of inefficiencies in high-level dynamic
languages (including the dynamic typing), and how they can be defeated. No prior knowledge of modern compiler technology is required.

About the presenter
----------------------------------------------------------------------------------------------------------------------------------------------------------------

**Alexey Protasov**

Alex is an enthusiastic independent developer with Russian origins living in Medellin, Colombia. He constantly dreams with "better" programming languages and
has over 30 years of experience with designing and implementing languages and development tools -- he had in the past a shareware visual programming tool,
worked in the area of compilers at Intel and Sun Microsystems, and taught compiler construction and the theory of formal languages at a university.

Alex speaks Spanish, English, and Russian. When he is not working on MANOOL, he likes swimming, traveling, and dancing (Salsa, Porro, Merengue, Cumbia, etc.).

What will the attendee learn? --- Comments for reviewers
----------------------------------------------------------------------------------------------------------------------------------------------------------------

Presentation outline:

* Introduction
  * About the author, the current status of MANOOL, references and contact details
  * When run-time performance is relevant and when it is not
  * Own middle-end/back-end (instead of LLVM)
  * No SSA (single static assignment form), as yet
* Language and compiler architecture
  * Scanner and parser
  * The compiler core dispatcher
  * Optimization and code generation pipeline
  * Object module linker/loader
* Intermediate representations
  * Abstract syntax tree
  * Compiled tree representation
  * Register machine (middle-end and back-end)
  * Object modules
* Sources of inefficiencies
  * Dynamic dispatch by type (dynamic overloading) and dynamic type checks
  * Dynamic specification for polymorphic operations and data member names
  * Value (un)boxing (pointer dereferencing)
  * Reference counting
  * Dynamic memory allocation (from heap)
* Middle-end (analysis and transformations)
  * Basic block predecessor analysis
  * Constant folding + inlining
  * Constant (including conditional) propagation
  * Copy propagation
  * Dead code elimination (producing dead values)
  * Unreachable code elimination
  * Jump optimizations (basic block merging, jump to jump, etc.)
  * Elimination of redundant load/store
  * Elimination of redundant reference counting
  * Elimination of redundant dynamic memory allocations
  * Miscellaneous: local algebraic simplification, canonicalization
* Back-end (code generation)
  * Instruction selection
  * Register allocation
* Test case (Game of Life)
  * Code
  * Optimizations
  * Benchmarking
* Conclusions
  * For MANOOL, even the most straightforward data and control flow analysis performs better than the best-in-the-class byte code/JIT VMs!


{%endraw%}{%include page_footer.md%}
