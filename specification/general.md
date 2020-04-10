---
# specification/general.md
title:   General &mdash; Specification
updated: 2019-12-21
---

<aside markdown="1" class="right">

See also:
  * [Syntax](/specification/core-language/syntax#start)
  * [Semantic Concepts](/specification/core-language/semantic-concepts#start)

</aside>

{%include spec_header.md%}{%raw%}


Definitions
----------------------------------------------------------------------------------------------------------------------------------------------------------------

To avoid any misunderstandings, let's agree on some important definitions that hold throughout this specification:

A _syntax_ of a formal language (such as a programming language) is a set of rules that determine how to discover a _syntactic structure_ of any phrase in that
language, where a syntactic structure of a phrase is a tree-like (i.e., hierarchical) structure induced on that phrase (in the form of a character string) that
enables formulation of the _meaning_ of the phrase (or a constituent phrase thereof) by means of a straightforward recursive definition.[^a1] Sometimes
different syntaxes may adequately describe the same programming language, and the syntax is normally specified by a context-free grammar plus lexical rules.

[^a1]: For instance, according to these definitions, the syntax of [Lisp]-family programming languages ([Common Lisp], [Scheme], [Clojure], [Kernel], etc.) is
       considered to be *exclusively* the syntax of S-expressions (sometimes called the surface or concrete syntax), regardless of any further (structural)
       requirements placed on the corresponding Lisp data (i.e., the abstract syntax trees), since in practice, the later provide sufficient guidance to deduce
       the meaning of a phrase in Lisp by *straightforward recursion* (anyway).

A _semantics_ of a formal language is a set of rules that determine how to discover the meaning of any phrase in that language.

Note that these definitions may be at odds with your intuition and may slightly differ from the corresponding definitions as they appear in some other contexts
(such as specifications of other programming languages, theoretic and applied linguistics, etc.).


General Program Structure
----------------------------------------------------------------------------------------------------------------------------------------------------------------

A program in MANOOL consists of one or more source files, referred to as _program units_ (or more accurately, _native program units_), each written in the
formal language of MANOOL [forms]. Thus, the MANOOL specification concerns, in fact, with syntactic structure and meaning of MANOOL forms (or equally, with
syntax and semantics of the language of forms).

A MANOOL program contains a designated _main program unit_ and all program units it depends on, either directly or indirectly (that is, recursively). The
presence of circular dependencies between program units would result in a meaningless program and even may be a cause of a non-terminating behavior thereof.

Note that program units written in MANOOL may also depend on _foreign program units_, implemented in other programming languages. For more information on
program units and their dependencies, refer to [Program Units].

[forms]:         /specification/core-language/semantic-concepts#h:forms-expressions-control-flow "Forms, expressions, control flow"
[Program Units]: /specification/core-language/program-units#start                                "Program Units"


The Abstract Machine
----------------------------------------------------------------------------------------------------------------------------------------------------------------

This specification introduces the concept of a fictional device implementing MANOOL, called the _abstract machine_, and in a few occasions its structure and
behavior are discussed explicitly. This is for illustration purposes only; by no means the MANOOL specification places requirements on either structure or
internal mode of operation of conforming implementations, which instead are to emulate the observable behavior of the abstract machine.[^c1] This in practice
extends to its asymptotic complexity characteristics whenever such characteristics are explicitly specified.

[^c1]: This principle is also known as the "as-if" principle.


Translation Overview
----------------------------------------------------------------------------------------------------------------------------------------------------------------

To figure out the meaning of a form that makes up a program unit, the abstract machine transforms (compiles) the contents of the source file into an internal
run-time representation, called _run-time code_.

Note that here the distinction between a compilation phase and a post-compilation (i.e., execution) phase is introduced not just for illustration purposes ---
in particular, some constituent [expressions] may actually need to be evaluated (once!) [during compilation] of the whole expression.[^d1] In this specification
a compilation phase is referred to hereinafter as _compile-time_ whereas a post-compilation phase as _run-time_.

[^d1]: Thus, strictly speaking, several compilation and execution phases may be interleaved in time.

A three-stage translation (i.e., compilation) scheme is suggested for the abstract machine:

* _lexical analysis_ --- The input string of characters is split into [lexical elements] (lexemes), whose meaning is then encoded in left-to-right order as a
  sequence of tokens.[^d2] Note that in practice, whatever internal syntactic structure of individual lexemes is devised, it is generally unimportant for
  determination of their meaning; rather, the lexical syntax is used for their sheer classification.

* _syntactic analysis_ --- The string of terminal symbols that corresponds to the sequence of tokens resulting from the previous compilation phase undergoes a
  [syntactic analysis] guided by a context-free grammar, which ultimately yields an _abstract syntax tree_ (AST) encoded as a MANOOL (semantic) [value]. Note
  that in contrast to lexical analysis, here syntactic structure is essential for correct interpretation of source code.

* _semantic analysis_ and _code generation_ --- The form and consistency (e.g., the presence and placement of certain keywords) of the AST resulting from the
  previous compilation phase are checked, and finally, the run-time code is produced.[^d3] Note that no new structural features are to be exposed on this stage,
  or they would at least reflect closely those of the AST.

[^d2]: Each of the tokens belongs to some class, encoded as a [terminal symbol] from the [syntactic analysis] standpoint, and has an optional semantic (MANOOL)
       [value].

[^d3]: In MANOOL (as opposed to other languages and with a notable exception of those based on the notation of S-expressions) many aesthetic aspects of source
       code that are traditionally examined on the syntactic analysis stage are irrelevant to the language syntax.

Semantic analysis and code generation is a compositional process; that is, to carry out the semantic analysis and code generation for a form (encoded in an
AST), the abstract machine performs (among other things) the semantic analysis and code generation for its constituent forms (represented by some subtrees of
the original AST). For a description of this process, refer to [Compiler Dispatcher].

[expressions]:         CoreSemantics#h:forms-expressions-control-flow  "Forms, expressions, control flow"
[during compilation]:  #
[terminal symbol]:     Syntax#h:metanotation                           "Metanotation"
[value]:               CoreSemantics#h:values-objects-special-entities "Values, objects, special entities"
[lexical elements]:    Syntax#h:lexical-structure                      "Lexical Structure"
[syntactic analysis]:  Syntax#h:syntactic-analysis                            "Syntactic Analysis"
[Compiler Dispatcher]: /specification/core-language/compiler-dispatcher#start


[Lisp]:         //en.wikipedia.org/wiki/Lisp_(programming_language)   "Wikipedia: Lisp"
[Common Lisp]:  //en.wikipedia.org/wiki/Common_Lisp                   "Wikipedia: Common Lisp"
[Scheme]:       //en.wikipedia.org/wiki/Scheme_(programming_language) "Wikipedia: Scheme"
[Clojure]:      //en.wikipedia.org/wiki/Clojure                       "Wikipedia: Clojure"
[Kernel]:       http://klisp.org                                      "klisp - a Kernel Programming Language implementation"

{%endraw%}{%include spec_footer.md%}
