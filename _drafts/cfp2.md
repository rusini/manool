---
title:   Native Run-time Performance for Dynamic Programming Languages
excerpt: Presentation talk for the [StrangeLoop](https://www.thestrangeloop.com) conference Sep-Oct'21 (CFP submission)
hidden:  false
---

{%include page_header.md%}


*{{page.excerpt}}*

Abstract
----------------------------------------------------------------------------------------------------------------------------------------------------------------

Much has been told about advantages and disadvantages of static vs dynamic type checking, without reaching a definitive conclusion. Being a low-budget project,
the programming language MANOOL has to seek major conceptual economy, leading to a compact implementation. For instance, MANOOL is a homoiconic language (like
Lisp or Tcl). On the other hand, its design and implementation simplicity is the most compelling reason for the dynamic typing choice.

Static type checking is often associated with languages that admit high-performance implementations (e.g., C or Java), but this does not always has to be the
case. For instance, sophisticated dynamic specialization (e.g., in case of JavaScript/V8 or LuaJIT) bridges the performance gap between static and dynamic
typing languages. However, such techniques may be prohibitively expensive for a small project like MANOOL, while being still insufficient to reach the highest
possible run-time performance (achievable for C/C++).

Despite of dynamic typing, MANOOL is specifically designed to admit effective static optimizations. While not a new idea (e.g., Julia has the necessary
properties for that), it seems that it has not been duly explored in mainstream programming yet (and hopefully, MANOOL has a more general-purpose design than
Julia).

You will learn about the overall architecture of MANOOL and its compiler, the intermediate representations, the sources of inefficiencies in high-level dynamic
languages (including the dynamic typing), and how they can be overcome. No prior knowledge of modern compiler technology is required.

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
  * When language implementation run-time performance is relevant and when it is not
  * Own middle-end/back-end rationale (instead of LLVM)
  * SSA (static single assignment form), to enable advanced data and control flow analysis
* Language and compiler architecture
  * Scanner and parser
  * The compiler core dispatcher
  * Optimization and code generation pipeline
  * Object module linker/loader (for separate compilation)
* Intermediate representations
  * Abstract syntax tree
  * Compiled tree representation
  * Register machine
  * Object modules (persistent cache)
* Sources of inefficiencies in high-level dynamic languages
  * Dynamic dispatch by type (dynamic overloading) and dynamic type checking
  * Dynamic specification of polymorphic operations and composite member names (in case of MANOOL)
  * Value (un)boxing (pointer dereferencing), reference counting, and dynamic memory allocation (from heap)
* Middle-end (analysis and transformations)
  * To-SSA transformation
  * Type inference via inter-procedural conditional constant propagation (IPSCCP, a bit more complete than in LLVM), inlining, constant folding
  * Copy propagation, dead code elimination, including unused and unreachable code
  * Jump optimizations (basic block merging and jump threading)
  * Aliasing analysis, elimination of redundant load/store, reference counting, dynamic memory allocations
  * Other analysis/transformations (GVN, CSE, PRE, strength reduction, loop unrolling, invariant code motion, autovectorization, etc.)
* Back-end (code generation)
  * Out-of-SSA transformation and register allocation
  * Instruction selection, peephole optimizations, etc.
* Test case (Game of Life)
  * Code
  * Optimizations
* Conclusions + Questions & Answers


{%include page_footer.md%}