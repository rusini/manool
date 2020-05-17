---
# Intro.md
permalink: /specification/introduction-to-manool
title:     Introduction to MANOOL
updated:   2020-01-09
---

{%include page_header.md%}{%raw%}


MANOOL is a [homoiconic][], [dynamic][], and [multi-paradigm][] [general-purpose][] [computer][] [programming][] [language][] with a [functional core][]. The
author's [implementation][] of MANOOL is made for [native-code][] [run-time][] [environments][], is written in *idiomatic* [C++11][] (with [GCC-specific][]
extensions), and currently runs under several [Unix-like][] [operating systems][] (OSes) on a number of [CPU][] [instruction-set architectures][] (ISAs). The
implementation is a [free and open-source][] (FOSS) [software][] [development][] [tool][] published under the version 3 of the [GNU][] [General Public
License][] (GPLv3).

[homoiconic]:                    //en.wikipedia.org/wiki/Homoiconic                           "Wikipedia: Homoiconic"
[dynamic]:                       //en.wikipedia.org/wiki/Dynamic_programming_language         "Wikipedia: Dynamic programming language"
[multi-paradigm]:                //en.wikipedia.org/wiki/Programming_paradigm                 "Wikipedia: Programming paradigm"
[general-purpose]:               //en.wikipedia.org/wiki/General-purpose_programming_language "Wikipedia: General-purpose programming language"
[computer]:                      //en.wikipedia.org/wiki/Computer                             "Wikipedia: Computer"
[programming]:                   //en.wikipedia.org/wiki/Computer_programming                 "Wikipedia: Computer programming"
[language]:                      //en.wikipedia.org/wiki/Programming_language                 "Wikipedia: Programming language"
[functional core]:               //en.wikipedia.org/wiki/Functional_programming               "Wikipedia: Functional programming"
[implementation]:                //en.wikipedia.org/wiki/Programming_language_implementation  "Wikipedia: Programming language implementation"
[native-code]:                   //en.wikipedia.org/wiki/Native_code                          "Wikipedia: Native code"
[run-time]:                      //en.wikipedia.org/wiki/Run_time_(program_lifecycle_phase)   "Wikipedia: Run time"
[environments]:                  //en.wikipedia.org/wiki/Runtime_environments                 "Wikipedia: Runtime environments"
[GCC-specific]:                  //en.wikipedia.org/wiki/GNU_Compiler_Collection              "Wikipedia: GNU Compiler Collection"
[Unix-like]:                     //en.wikipedia.org/wiki/Unix-like                            "Wikipedia: Unix-like"
[operating systems]:             //en.wikipedia.org/wiki/Operating_systems                    "Wikipedia: Operating systems"
[CPU]:                           //en.wikipedia.org/wiki/Central_processing_unit              "Wikipedia: Central processing unit"
[instruction-set architectures]: //en.wikipedia.org/wiki/Instruction_set_architectures        "Wikipedia: Instruction set architectures"
[free and open-source]:          //en.wikipedia.org/wiki/Free_and_open-source                 "Wikipedia: Free and open-source"
[software]:                      //en.wikipedia.org/wiki/Software                             "Wikipedia: Software"
[development]:                   //en.wikipedia.org/wiki/Software_development                 "Wikipedia: Software development"
[tool]:                          //en.wikipedia.org/wiki/Development_tool                     "Wikipedia: Development tool"
[GNU]:                           //en.wikipedia.org/wiki/GNU_Project                          "Wikipedia: GNU Project"
[General Public License]:        //en.wikipedia.org/wiki/General_Public_License               "Wikipedia: General Public License"


Purpose
----------------------------------------------------------------------------------------------------------------------------------------------------------------

MANOOL is characterized as a _general-purpose_ language because it is *not* specific to a particular [problem domain][], and it is instead broadly applicable
across several problem domains. However, it is meant to compare and compete directly with such programming languages as [Python], [PHP], [Ruby], [Perl], and
[Tcl][] (i.e., so-called [scripting languages][]), or even [Scheme] and [Common Lisp], which somehow dooms its purpose.

In overall, MANOOL is a practical language: it is conceived as a tool useful in the professional field of programming rather than as a [proof of concept][] for
any new programming techniques or mechanisms, although incidentally its [syntax][] and [semantics][] do have a combination of unusual features.

[problem domain]:      //en.wikipedia.org/wiki/Problem_domain                 "Wikipedia: Problem domain"
[scripting languages]: //en.wikipedia.org/wiki/Scripting_languages            "Wikipedia: Scripting languages"
[proof of concept]:    //en.wikipedia.org/wiki/Proof_of_concept               "Wikipedia: Proof of concept"
[syntax]:              //en.wikipedia.org/wiki/Syntax_(programming_languages) "Wikipedia: Syntax"
[semantics]:           //en.wikipedia.org/wiki/Semantics_(computer_science)   "Wikipedia: Semantics"


Motivation
----------------------------------------------------------------------------------------------------------------------------------------------------------------

The project MANOOL emerged out of mere programming language enthusiasm and frustration with existing languages but is based on ideas and experience its author
acquired throughout more than 25 years. Its design and implementation is the result of about 3.5 years of almost full-time work.


Examples
----------------------------------------------------------------------------------------------------------------------------------------------------------------

The following ["Hello, world!" program][] provides a basic idea of what [programs][] in MANOOL may look like:

    {{extern "manool.org.18/std/0.5/all"} in WriteLine[Out; "Hello, world!"]}

And the following MANOOL [expression][] evaluates to a ([recursive][]) MANOOL [procedure][] (a function in more conventional terminology) used to calculate the
[factorial][] of an [integral][] [argument][]:

    { let rec
      { Fact =
        { proc { N } as
        : if N == 0 then 1 else
          N * Fact[N - 1]
        }
      }
      in
      Fact
    }

It is worth to observe that while [source code][] in MANOOL may look at first sight unfamiliar or even awkward, there are really good reasons behind it since
the syntax of MANOOL is a result of many design trade-offs. However, as in the case of [Smalltalk], the syntax of MANOOL is actually so simple that it could be
"described on a postcard".

["Hello, world!" program]: //en.wikipedia.org/wiki/%22Hello,_World!%22_program     "Wikipedia: &ldquo;Hello, World!&rdquo; program"
[programs]:                //en.wikipedia.org/wiki/Computer_programs               "Wikipedia: Computer programs"
[expression]:              //en.wikipedia.org/wiki/Expression_(computer_science)   "Wikipedia: Expression"
[recursive]:               //en.wikipedia.org/wiki/Recursive_(computer_science)    "Wikipedia: Recursive"
[procedure]:               //en.wikipedia.org/wiki/Procedure_(computer_science)    "Wikipedia: Procedure"
[factorial]:               //en.wikipedia.org/wiki/Factorial                       "Wikipedia: Factorial"
[integral]:                //en.wikipedia.org/wiki/Integer                         "Wikipedia: Integer"
[argument]:                //en.wikipedia.org/wiki/Argument_(computer_programming) "Wikipedia: Argument"
[source code]:             //en.wikipedia.org/wiki/Source_code                     "Wikipedia: Source code"


Goals
----------------------------------------------------------------------------------------------------------------------------------------------------------------

MANOOL has *explicitly* stated design goals (ordered according to priority), which are
  1. [implementation simplicity][] (which is the sole most important consideration in the design);
  2. [expressive power][] (in practical sense), [usability][], and general utility (value for consumers); attention to syntax and semantics details;
  3. [correctness][], [security][], and overall [quality][] of implementation; [run-time reliability][];
  4. [run-time performance][] and [scalability][]; and
  5. consistency, completeness, [orthogonality][] of features and language [elegance][]; [conceptual economy][] and purity.

The project MANOOL strives to satisfy *all* of the above goals (in that order) and do it *better* than existing alternatives would do!

Note that quality of [diagnostics][] and [coding defect][] prevention have been deliberately left, however, among low-priority goals.

[implementation simplicity]: //en.wikipedia.org/wiki/Worse_is_better                     "Wikipedia: Worse is better"
[expressive power]:          //en.wikipedia.org/wiki/Expressive_power_(computer_science) "Wikipedia: Expressive power"
[usability]:                 //en.wikipedia.org/wiki/Usability                           "Wikipedia: Usability"
[correctness]:               //en.wikipedia.org/wiki/Correctness_(computer_science)      "Wikipedia: Correctness"
[security]:                  //en.wikipedia.org/wiki/Secure_coding                       "Wikipedia: Secure coding"
[quality]:                   //en.wikipedia.org/wiki/Software_quality                    "Wikipedia: Software quality"
[run-time reliability]:      //en.wikipedia.org/wiki/Reliability_engineering             "Wikipedia: Reliability engineering"
[run-time performance]:      //en.wikipedia.org/wiki/Benchmark_(computing)               "Wikipedia: Benchmark"
[scalability]:               //en.wikipedia.org/wiki/Scalability                         "Wikipedia: Scalability"
[orthogonality]:             //en.wikipedia.org/wiki/Orthogonality_(programming)         "Wikipedia: Orthogonality"
[elegance]:                  //en.wikipedia.org/wiki/Elegance                            "Wikipedia: Elegance"
[conceptual economy]:        //en.wikipedia.org/wiki/Occam%27s_razor                     "Wikipedia: Occam's razor"
[diagnostics]:               //en.wikipedia.org/wiki/Diagnostics                         "Wikipedia: Diagnostics"
[coding defect]:             //en.wikipedia.org/wiki/Software_defect                     "Wikipedia: Software defect"


Results
----------------------------------------------------------------------------------------------------------------------------------------------------------------

### Implementation simplicity
The MANOOL [translator][] fits in less than 10K [lines of code][] (KLOC) in C++11 plus less than 500 LOC in MANOOL, which covers the core language as well as
the [standard][] [library][].

[translator]:    //en.wikipedia.org/wiki/Translator_(computing) "Wikipedia: Translator"
[lines of code]: //en.wikipedia.org/wiki/Lines_of_code          "Wikipedia: Lines of code"
[standard]:      //en.wikipedia.org/wiki/Standard_library       "Wikipedia: Standard library"
[library]:       //en.wikipedia.org/wiki/Library_(computing)    "Wikipedia: Library"

### Expressive power
MANOOL is directly suitable for accurately expressing [algorithms][] (*up to* [asymptotic computational complexity][]) as complex as those that one might find
in [computer-science][] (CS) papers where mostly some form of [high-level][] [pseudocode][] is encountered.

[algorithms]:                          //en.wikipedia.org/wiki/Algorithms                          "Wikipedia: Algorithms"
[asymptotic computational complexity]: //en.wikipedia.org/wiki/Asymptotic_computational_complexity "Wikipedia: Asymptotic computational complexity"
[computer-science]:                    //en.wikipedia.org/wiki/Computer_science                    "Wikipedia: Computer science"
[high-level]:                          //en.wikipedia.org/wiki/High-level_programming_language     "Wikipedia: High-level programming language"
[pseudocode]:                          //en.wikipedia.org/wiki/Pseudocode                          "Wikipedia: Pseudocode"

### Run-time performance
According to simple synthetic [benchmarks][], the MANOOL implementation executes programs notably faster than some competitors and slightly faster or on a par
with other competitors.

[benchmarks]: //en.wikipedia.org/wiki/Benchmark_(computing) "Wikipedia: Benchmark"

### Run-time reliability
When needed, MANOOL programs can be instructed to recover even from [dynamic memory][] (heap and stack) exhaustion.

[dynamic memory]: //en.wikipedia.org/wiki/Dynamic_memory "Wikipedia: Dynamic memory"

### Other features
* Very compact (or even [minimalist][]) core language (up to a point where a [meta-circular specification][] might be appropriate)
* Convenient standard library (but completely *optional* to use)
* Computational primitives based on [Church's][] [&lambda;-calculus][] (in the spirit of [Landin's][] [ISWIM] prototype language/[ML]-like languages)
    - [Name bindings][] with [static (lexical) scope][]
    - *Explicit* [variable capture][] and classification of name bindings into [compile][compile-time]- and run-time
    - Mainly [eager][] (strict) [evaluation strategy][] (from the perspective of &lambda;-calculus) with possible [side effects][]
* [Compile-time evaluation][]
* [Metaprogramming][]:
    - [Lisp]-like [syntactic macros][] with optional [macro hygiene][]
    - (optionally) [self-modifying code][]
* [Block-structured][] and [expression-oriented][] (from the perspective of [procedural][] [imperative programming][])
* [Dynamic][dynamic typing] (latent) but [strong][strong typing] [data typing][] [discipline][typing discipline]
* [Ad-hoc][ad-hoc polymorphism] [polymorphism][]:
    - run-time [function][function overload]/[operator][operator overload] overload resolution (via [dynamic single-dispatch][])
* *Observably* (modulo timings) [non-referential][] (by-value) data model encouraging (but not requiring) to use (observably) [immutable][] [objects][] using
  [automatic reference counting][] (ARC) and [transparent copy-on-write][] (COW) implementation techniques
* [Move semantics][] and [syntactic sugar][] that *emulates* [in-place][in-place updates] partial [updates][]
* [Very high-level][] [composite][composite types] [abstract data types][] (ADTs):
    - [set-theoretic operations][], [comprehensions][], and [logic quantifications][] inspired by the [math notation][] and [SETL]
    - values of *any* types (as long as the former are [totally ordered][]) can be used as set elements and map keys
    - [iterators][] with elements of [lazy evaluation strategy][]
* [Modular programming][]:
    - [namespaces][]
    - name binding visibility control
    - multiple [source files][] (plus support for [Ada]-like private program units)
* User-defined abstract data types:
    - [data encapsulation][] (with visibility control)
* [Exception handling][] (with stack unwinding)
* [Decimal][] [floating-point arithmetic][] ([out-of-the-box][])
* [Multithreading-aware][] implementation, free from [global interpreter lock][] (GIL)
* Simple [plug-in][] [application programming interface][] (API)

[minimalist]:                        //en.wikipedia.org/wiki/Minimalism_(computing)                   "Wikipedia: Minimalism"
[meta-circular specification]:       //en.wikipedia.org/wiki/Meta-circular_evaluator                  "Wikipedia: Meta-circular evaluator"
[Church's]:                          //en.wikipedia.org/wiki/Alonzo_Church                            "Wikipedia: Alonzo Church"
[&lambda;-calculus]:                 //en.wikipedia.org/wiki/&lambda;-calculus                        "Wikipedia: &lambda;-calculus"
[Landin's]:                          //en.wikipedia.org/wiki/Peter_Landin                             "Wikipedia: Peter Landin"
[Name bindings]:                     //en.wikipedia.org/wiki/Name_binding                             "Wikipedia: Name binding"
[static (lexical) scope]:            //en.wikipedia.org/wiki/Static_scope                             "Wikipedia: Static scope"
[variable capture]:                  //en.wikipedia.org/wiki/Closure_(computer_programming)           "Wikipedia: Closure"
[compile-time]:                      //en.wikipedia.org/wiki/Compile-time                             "Wikipedia: Compile-time"
[eager]:                             //en.wikipedia.org/wiki/Eager_evaluation                         "Wikipedia: Eager evaluation"
[evaluation strategy]:               //en.wikipedia.org/wiki/Evaluation_strategy                      "Wikipedia: Evaluation strategy"
[side effects]:                      //en.wikipedia.org/wiki/Side_effect_(computer_science)           "Wikipedia: Side effect"
[Compile-time evaluation]:           //en.wikipedia.org/wiki/General_constant_expressions             "Wikipedia: General constant expressions"
[Metaprogramming]:                   //en.wikipedia.org/wiki/Metaprogramming                          "Wikipedia: Metaprogramming"
[syntactic macros]:                  //en.wikipedia.org/wiki/Macro_(computer_science)                 "Wikipedia: Macro"
[macro hygiene]:                     //en.wikipedia.org/wiki/Hygienic_macro                           "Wikipedia: Hygienic macro"
[self-modifying code]:               //en.wikipedia.org/wiki/Self-modifying_code                      "Wikipedia: Self-modifying code"
[Block-structured]:                  //en.wikipedia.org/wiki/Block_(programming)                      "Wikipedia: Block"
[expression-oriented]:               //en.wikipedia.org/wiki/Expression-oriented_programming_language "Wikipedia: Expression-oriented programming language"
[procedural]:                        //en.wikipedia.org/wiki/Procedural_programming                   "Wikipedia: Procedural programming"
[imperative programming]:            //en.wikipedia.org/wiki/Imperative_programming                   "Wikipedia: Imperative programming"
[dynamic typing]:                    //en.wikipedia.org/wiki/Latent_typing                            "Wikipedia: Latent typing"
[strong typing]:                     //en.wikipedia.org/wiki/Strong_typing                            "Wikipedia: Strong typing"
[data typing]:                       //en.wikipedia.org/wiki/Data_typing                              "Wikipedia: Data typing"
[typing discipline]:                 //en.wikipedia.org/wiki/Type_system                              "Wikipedia: Type system"
[ad-hoc polymorphism]:               //en.wikipedia.org/wiki/Ad-hoc_polymorphism                      "Wikipedia: Ad-hoc polymorphism"
[polymorphism]:                      //en.wikipedia.org/wiki/Polymorphism_(computer_science)          "Wikipedia: Polymorphism"
[function overload]:                 //en.wikipedia.org/wiki/Function_overloading                     "Wikipedia: Function overloading"
[operator overload]:                 //en.wikipedia.org/wiki/Operator_overloading                     "Wikipedia: Operator overloading"
[dynamic single-dispatch]:           //en.wikipedia.org/wiki/Dynamic_dispatch                         "Wikipedia: Dynamic dispatch"
[non-referential]:                   //en.wikipedia.org/wiki/Reference_(computer_science)             "Wikipedia: Reference"
[immutable]:                         //en.wikipedia.org/wiki/Immutable_object                         "Wikipedia: Immutable object"
[objects]:                           //en.wikipedia.org/wiki/Object_(computer_science)                "Wikipedia: Object"
[automatic reference counting]:      //en.wikipedia.org/wiki/Reference_counting                       "Wikipedia: Reference counting"
[transparent copy-on-write]:         //en.wikipedia.org/wiki/Copy-on-write                            "Wikipedia: Copy-on-write"
[Move semantics]:                    //en.wikipedia.org/wiki/C%2B%2B11                                "Wikipedia: C++11"
[syntactic sugar]:                   //en.wikipedia.org/wiki/Syntactic_sugar                          "Wikipedia: Syntactic sugar"
[in-place updates]:                  //en.wikipedia.org/wiki/Persistent_data_structure                "Wikipedia: Persistent data structure"
[updates]:                           //en.wikipedia.org/wiki/Assignment_(computer_science)            "Wikipedia: Assignment"
[Very high-level]:                   //en.wikipedia.org/wiki/Very_high-level_programming_language     "Wikipedia: Very high-level programming language"
[composite types]:                   //en.wikipedia.org/wiki/Composite_data_type                      "Wikipedia: Composite data type"
[abstract data types]:               //en.wikipedia.org/wiki/Abstract_data_types                      "Wikipedia: Abstract data types"
[set-theoretic operations]:          //en.wikipedia.org/wiki/Set_theoretic_programming                "Wikipedia: Set theoretic programming"
[comprehensions]:                    //en.wikipedia.org/wiki/List_comprehensions                      "Wikipedia: List comprehensions"
[logic quantifications]:             //en.wikipedia.org/wiki/Quantification_(logic)                   "Wikipedia: Quantification"
[math notation]:                     //en.wikipedia.org/wiki/Set-builder_notation                     "Wikipedia: Set-builder notation"
[totally ordered]:                   //en.wikipedia.org/wiki/Totally_ordered                          "Wikipedia: Totally ordered"
[iterators]:                         //en.wikipedia.org/wiki/Iterators                                "Wikipedia: Iterators"
[lazy evaluation strategy]:          //en.wikipedia.org/wiki/Lazy_evaluation                          "Wikipedia: Lazy evaluation"
[Modular programming]:               //en.wikipedia.org/wiki/Modular_programming                      "Wikipedia: Modular programming"
[namespaces]:                        //en.wikipedia.org/wiki/Namespaces                               "Wikipedia: Namespaces"
[source files]:                      //en.wikipedia.org/wiki/Source_files                             "Wikipedia: Source files"
[data encapsulation]:                //en.wikipedia.org/wiki/Encapsulation_(computer_programming)     "Wikipedia: Encapsulation"
[Exception handling]:                //en.wikipedia.org/wiki/Exception_handling                       "Wikipedia: Exception handling"
[Decimal]:                           //en.wikipedia.org/wiki/Decimal_floating-point                   "Wikipedia: Decimal floating-point"
[floating-point arithmetic]:         //en.wikipedia.org/wiki/Floating-point_arithmetic                "Wikipedia: Floating-point arithmetic"
[out-of-the-box]:                    //en.wikipedia.org/wiki/Out_of_the_box_(feature)                 "Wikipedia: Out of the box"
[Multithreading-aware]:              //en.wikipedia.org/wiki/Thread_(computing)                       "Wikipedia: Thread"
[global interpreter lock]:           //en.wikipedia.org/wiki/Global_interpreter_lock                  "Wikipedia: Global interpreter lock"
[plug-in]:                           //en.wikipedia.org/wiki/Plug-in_(computing)                      "Wikipedia: Plug-in"
[application programming interface]: //en.wikipedia.org/wiki/Application_programming_interface        "Wikipedia: Application programming interface"


Influences
----------------------------------------------------------------------------------------------------------------------------------------------------------------

The following programming languages have driven major inspiration for MANOOL (in alphabetical order):
  * [Ada], [APL]
  * [C++], [Clojure], [CLU], [Common Lisp]
  * [Forth]
  * [Haskell]
  * [ISWIM]
  * [Kernel][] (<http://klisp.org>)
  * [Modula-2]
  * [Oberon]
  * [Pascal], [Perl], [Python]
  * [Scheme], [Self], [SETL2], [Smalltalk], [Standard ML]
  * [Tcl]

  And the following programming languages are noteworthy anti-influences:
  * [C#], [Java], [JavaScript], [PHP]

[Ada]:         //en.wikipedia.org/wiki/Ada_(programming_language)     "Wikipedia: Ada"
[APL]:         //en.wikipedia.org/wiki/APL_(programming_language)     "Wikipedia: APL"
[C++]:         //en.wikipedia.org/wiki/C%2B%2B                        "Wikipedia: C++"
[C++11]:       //en.wikipedia.org/wiki/C%2B%2B11                      "Wikipedia: C++11"
[Clojure]:     //en.wikipedia.org/wiki/Clojure                        "Wikipedia: Clojure"
[CLU]:         //en.wikipedia.org/wiki/CLU_(programming_language)     "Wikipedia: CLU"
[Common Lisp]: //en.wikipedia.org/wiki/Common_Lisp                    "Wikipedia: Common Lisp"
[Forth]:       //en.wikipedia.org/wiki/Forth_(programming_language)   "Wikipedia: Forth"
[Haskell]:     //en.wikipedia.org/wiki/Haskell_(programming_language) "Wikipedia: Haskell"
[ISWIM]:       //en.wikipedia.org/wiki/ISWIM                          "Wikipedia: ISWIM"
[Kernel]:      //en.wikipedia.org/wiki/Kernel_(programming_language)  "Wikipedia: Kernel"
[Lisp]:        //en.wikipedia.org/wiki/Lisp_(programming_language)    "Wikipedia: Lisp"
[ML]:          //en.wikipedia.org/wiki/ML_(programming_language)      "Wikipedia: ML"
[Modula-2]:    //en.wikipedia.org/wiki/Modula-2                       "Wikipedia: Modula-2"
[Oberon]:      //en.wikipedia.org/wiki/Oberon_(programming_language)  "Wikipedia: Oberon"
[Pascal]:      //en.wikipedia.org/wiki/Pascal_(programming_language)  "Wikipedia: Pascal"
[Perl]:        //en.wikipedia.org/wiki/Perl                           "Wikipedia: Perl"
[Python]:      //en.wikipedia.org/wiki/Python_(programming_language)  "Wikipedia: Python"
[Ruby]:        //en.wikipedia.org/wiki/Ruby_(programming_language)    "Wikipedia: Ruby"
[Scheme]:      //en.wikipedia.org/wiki/Scheme_(programming_language)  "Wikipedia: Scheme"
[Self]:        //en.wikipedia.org/wiki/Self_(programming_language)    "Wikipedia: Self"
[SETL]:        //en.wikipedia.org/wiki/SETL                           "Wikipedia: SETL"
[SETL2]:       //en.wikipedia.org/wiki/SETL                           "Wikipedia: SETL"
[Smalltalk]:   //en.wikipedia.org/wiki/Smalltalk                      "Wikipedia: Smalltalk"
[Standard ML]: //en.wikipedia.org/wiki/Standard_ML                    "Wikipedia: Standard ML"
[Tcl]:         //en.wikipedia.org/wiki/Tcl                            "Wikipedia: Tcl"
[C#]:          //en.wikipedia.org/wiki/C_Sharp_(programming_language) "Wikipedia: C#"
[Java]:        //en.wikipedia.org/wiki/Java_(programming_language)    "Wikipedia: Java"
[JavaScript]:  //en.wikipedia.org/wiki/JavaScript                     "Wikipedia: JavaScript"
[PHP]:         //en.wikipedia.org/wiki/PHP                            "Wikipedia: PHP"


{%endraw%}{%include page_footer.md%}

<aside markdown="1">
See Also
========
+ [Examples](/tutorial/lesson-1) -- tutorial
+ [Online Evaluator](/eval) -- to execute short programs in MANOOL and examine results without actually installing or compiling anything yourself
+ [Source Code Repository](//github.com/rusini/manool "to GitHub&hellip;") -- browse, download, clone, or fork
+ [Releases](//github.com/rusini/manool/releases "to GitHub&hellip;") -- to download pre-compiled binaries
+ <info@manool.org> -- contact email address
</aside>
