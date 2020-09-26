---
title:   MANOOL --- Practical Language with Universal Syntax and Only Library-Level Features (Except One)
updated: 2020-01-14
excerpt: >-
    Have you ever said to yourself?: "This new version of _my favorite programming language_ is great, but why on earth did they introduce the feature _X_? I
    don't need it. The language has become now more bloated than ever!"
---
{%include post_header.md%}{%raw%}


First, I have to make two disclaimers:

In this article I do *not* intend to propose "the next greatest" minimalist Turing-complete programming language; we already have at least two of them: [Iota
and Jot] (based on combinatory logic)[^a1]. These are extreme, "esoteric" examples that show that language minimalism itself may be a useless, even absurd
design goal from the practical standpoint. So, this article focuses on something more practical --- I rather try to show that (a kind of) minimalism may help to
achieve *other* attractive goals.

[^a1]: With the same success, we could just view *any* existing Turing-complete programming language _L_ as having only two operators --- `0` and `1` --- `0`
       appends the digit "0" to a buffer, whereas `1` appends "1", and every eight digits the abstract machine feeds the corresponding octet to a translator of
       _L_, et voil&agrave;.

This article talks about *simple*, cost-effective solutions in the spirit of MANOOL design, easy to implement and easy to master, although they may be a bit
incomplete (which is aligned with the _Pareto principle_, also known as the _80/20 rule_). Thus, I do not pretend to make scientifically rigorous claims here
since ultimately, simplicity is difficult to formalize.

In other words, although I strove to align the article and the entire MANOOL design with the established theoretic computer science formalisms and terminology,
this article is *not* about computer science but rather software engineering arts. However, it *does* talk (hopefully, in an entertaining form) about how some
phenomena and connections between them predetermine some key aspects of the architecture of the programming language MANOOL (thus, this article is hopefully
neither a goal-oriented technical manual nor a meaningless advertisement).

Also, to avoid any misunderstandings, let's agree on some important definitions that hold throughout this article:

> A _syntax_ of a formal language (such as a programming language) is a set of rules that determine how to discover a _syntactic structure_ of any phrase in
> that language, where a syntactic structure of a phrase is a tree-like (i.e., hierarchical) structure induced on that phrase (in the form of a character
> string) that enables formulation of the _meaning_ of the phrase (or a constituent phrase thereof) by means of a straightforward recursive definition.[^a2]
> Sometimes different syntaxes may adequately describe the same programming language, and the syntax is normally specified by a context-free grammar plus
> lexical rules.
>
> [^a2]: For instance, according to these definitions, the syntax of [Lisp]-family programming languages ([Common Lisp], [Scheme], [Clojure], [Kernel], etc.) is
>        considered to be *exclusively* the syntax of S-expressions (sometimes called the surface or concrete syntax), regardless of any further (structural)
>        requirements placed on the corresponding Lisp data (i.e., the abstract syntax trees), since in practice, the later provide sufficient guidance to
>        deduce the meaning of a phrase in Lisp by *straightforward recursion* (anyway).
>
> A _semantics_ of a formal language is a set of rules that determine how to discover the meaning of any phrase in that language.

(See [Definitions](/specification/general#h:definitions "MANOOL Specification: Definitions"))

Note that these definitions may be at odds with your intuition and may slightly differ from the corresponding definitions as they appear in some other contexts
(such as specifications of other programming languages, theoretic and applied linguistics, etc.).

And now, let's start ...


General
----------------------------------------------------------------------------------------------------------------------------------------------------------------

### The problem --- feature bloat ######################################################################################

Have you ever said to yourself?:
> This new version of _my favorite programming language_ is great, but why on earth did they introduce the feature _X_? I don't need it. The language has become
> now more bloated than ever!

or
> This new version of _my favorite programming language_ is great, but it would be even better if they introduced the feature _Y_.

See the pattern? --- In one case you're complaining about the introduction of a new feature _X_, whereas in another case you're doing (exactly) the *opposite*:
requesting a new feature _Y_ (which might happen to be the feature _X_ from another user's standpoint).

And since we all have different problems at hand and therefore different needs, in the course of programming language evolution, satisfying the needs of some
language users eventually implies disappointing other users (those who request new features as well). So, can we break this vicious circle?

### Summary --- solution in MANOOL #####################################################################################

The answer of MANOOL to the above question is "Yes, we can!". For instance, in MANOOL there are
  * *no* conditionals (`if ... then`, ...) built into the language core,
  * *no* &lambda;-expressions (`proc ... as`, ...),
  * *no* binding constructs (`let ... in`, ...),
  * *no* floating-point literals (`F64[...]$`, ...), etc.;

  instead, all of the above are just _library features_ (like, say, standard mathematical functions). You normally *import* them *explicitly* from a standard
library _module_ into the current _binding environment_ in order to be able to use them, but this is completely optional --- with the same success, you can
write and import from your own similar modules instead, and as the language evolves, different standard library _versions_ can be easily provided (based on the
same code base). In the same way, several language _dialects_ and/or _sublanguages_ (including _domain-specific_ ones) can be provided in the form of library
modules and can even be *mixed* in the same region of the program unit (provided all naming conflicts, if any, are resolved somehow) or used separately from
within different regions.

Incidentally, the *only* built-in, predefined construct in MANOOL is `extern`, which is used to reference some external _entity_ (i.e., either a _first-class_
value or a special, _non-value_ entity, such as a standard library module, or a special, non-value entity that the keyword `if` denotes). Actually, there are
also some _core features_, like
  * the ability to express at least some values literally,
  * the ability to use infix operators and to explicitly group subexpressions syntactically,
  * the ability to call procedures with arguments, etc.;

  however, they
  * are very, very basic and much more abstract,
  * are not even required to form a full basis for a Turing-complete programming language, and
  * are expected to evolve *very, very slowly*, if at all.

Of course, in practice, any such system inevitably has its *limitations* and *scope of applicability*, since every programming language is a result of many
design trade-offs.[^b1] Also, this is not a completely new approach --- there are some related studies in theoretic computer science, and there were a few
intents to design *other* programming languages with library-only functionality (sometimes, but not always, qualified as _extensible_).[^b2] Unfortunately, *no*
such language can be considered mainstream nowadays (and I *do* aim MANOOL to be a practical tool and maybe to become kind of mainstream some day); thus, I
invite you to discover *yourself* whether the approach and especially the limitations of MANOOL make sense for you.

[^b1]: Strictly speaking, no Turing-complete language _L_ has such limitations since we could always implement in the language _L_ another Turing-complete
       language _L'_ that would lack limitations from any chosen set of limitations, but I am talking here about cost-effective use cases only (i.e., in
       practical sense).

[^b2]: For instance, at least one dialect of the programming language Scheme provides almost empty top-level binding environment by default. But MANOOL is not
       Scheme --- although it has a similar architecture, it has at least a distinct syntax and a distinct data model, among other things.

The MANOOL approach involves three aspects:
  * a [universal syntax] --- an analog of the syntax of S-expressions (in Lisp-family programming languages);
  * a [module system] --- a set of mutually orthogonal features provided (in turn) mainly by the standard library; and
  * a (very) simple and compact semantic analysis and code generation [core dispatcher].

Further in this article, I explain what a universal syntax *is* and provide a few examples of programs in MANOOL for illustration purposes, although
unfortunately, the article has *no* room for a detailed description of the MANOOL syntax as such, its advantages, drawbacks and/or the rationale (apart from a
simple reproduction of the grammar). Also, I provide some examples that support the above claims about feature _composability_ and _modularity_ of the language
but do *not* provide a detailed description of the MANOOL module system. Both topics are two whole new stories that deserve separate articles, which I'll
publish in the future, so if you are curious, stay tuned ...

I also touch in this article the closely [related subjects][homoiconicity] of homoiconicity, metaprogramming, and syntactic macros (for the case of MANOOL).

### Further motivation #################################################################################################

Let's see what may happen if we just put up with language bloat ...

Let's assume that C++98 is an *extension* of C. Remember the famous issue with the C++ grammar? --- What does the following declaration in C++ mean?:

> _t_<sub>1</sub> _a_`(`_t_<sub>2</sub>`()``)``;`

  Is _a_ declared as an object of type _t_<sub>1</sub> and initialized with the value of a value-initialized object of type _t_<sub>2</sub>, or is it a function
returning a result of type _t_<sub>1</sub> and taking as an argument a pointer to a function without parameters returning a result of type _t_<sub>2</sub>?

The above code exposes a parsing conflict. Of course, the C++ specification still tells us that _a_ *is a function*, but this is because the specification has
additional conflict-resolution provisions; in fact, no (Chomsky-style)[^b3] grammar can unambiguously capture the distinction between the two situations (_a_ is
an object vs. _a_ is a function). And even though we can always resolve parsing conflicts in some way, I argue that the resulting language may become then
needlessly *irregular*, *inconsistent*, *complex*, and *unintuitive* due to additional (ad hoc) conflict-resolution rules.

[^b3]: Some grammar formalisms distinct from Chomsky's grammars intrinsically support conflict-resolution policies, but I argue that this does *not* solve the
       problems we are talking about here.

But *how often* the above situation arises in practice? --- Well, the C++ standard committee made this mistake in C++98 and suggested a fix in C++11 via the
uniform initialization notation. Also, while I was playing around with LALR(1)-grammars for MANOOL, I realized that it was too easy to accidentally introduce
grammar ambiguities (especially if we are short of terminal symbols to play with). It is even more easy to come to a grammar that is unambiguous but does not
fall into the (cost-effective) LALR(1) class (which is undesirable).

The above illustrates that we should *not* ignore language bloat (which is inherently unavoidable) since it may cause real problems and may be harmful if we
do not mitigate the related issues somehow. Besides, the situation would be even worse if instead of language evolution and extensibility, we were talking about
modularity and composability (out of sublanguages).

### State of the art ###################################################################################################

There are usually two kinds of features in a programming language:
  * those that correspond to some grammar production (such as conditionals and name bindings) and
  * those that have a name in the standard library (such as standard mathematical functions).

  As shown above, _syntactic conflicts_ are tricky to avoid and tricky to manage. On the other hand, _naming conflicts_ can be more plausibly resolved via name
spacing and/or aliasing. So, what if we could *unify* both kinds of features and provide *all of them* just using names (or as they say, bind them to names)?

There is at least one language that excels at such unification, [Kernel],[^b4] but the problem with Kernel is that its computation model seems to be almost
inevitably inefficient because in Kernel actions that are otherwise performed *once*, during what would be called a compilation phase,[^b5] are performed
*repeatedly*, every time an expression is evaluated (in the course of program execution).

[^b4]: Other possible examples are [Forth], [Tcl], and other dialects of [Lisp], but Kernel may be the most sophisticated of them. Besides, Forth is too
       low-level and Tcl leads to inevitably inefficient implementations.

[^b5]: ... such as checking of presence and placement of certain syntactic structures, (almost unavoidable) linear-time lookup of identifiers, etc. ...

A simple solution to that problem consists in designing a language in such a way that such activities are explicitly factored out into a separate compilation
phase. And here we come to a translation/execution scheme that resembles in many ways the usual evaluation model of any dialect of Lisp.[^b6] Compared to
Kernel, a (small) price is that in some cases names are now bound to _second-class_ entities (which cannot be freely operated with at run-time), but we still
enjoy a unified approach (at 80%).

[^b6]: They say that whoever does not understand Lisp is doomed to reinvent it ... Still, MANOOL does differ from all existing Lisp-family programming languages
       in a number of important aspects.

Note that there also exist quite a few studies about general context-free grammar composability, although MANOOL is based on a simpler approach to the problem
of feature composition (see also [Background theory]).

### Example ############################################################################################################

Almost every working example from the MANOOL [tutorial](/tutorial/ "MANOOL Tutorial") contains near its beginning an instance of the `extern` construct (i.e.,
`{extern "..."}`), e.g.:[^b7]

    {{extern "manool.org.18/std/0.6/all"} in Out.WriteLine["Hello, world!"]}

  where the standard library module is identified by a special external entity _path_ specified as a compile-time expression that evaluates to a string (i.e.,
`"manool.org.18/..."`). As you may note, a global name hierarchy is proposed based on the global Internet domain name system, which is a convention for some
other programming languages (here, `18` means the year of manool.org domain registration --- 2018).

[^b7]: All examples in MANOOL presented in this article are complete, working examples that you can submit to the
       [online evaluator](/eval "MANOOL Online Evaluator") to test.

The importing part is `... in ...`, which is used here to "apply" the referenced external module (a standard library module in this case) to the body expression
after `in`, making that expression to be compiled in the binding environment augmented with all name bindings the module provides. When evaluated, the body
expression will transmit the `"Hello, world!"` string to the standard output stream of the current process.

In fact, the *only* standard library member used in the previous example is `Out`, which represents the standard output. `WriteLine` here is *not* a
library-supplied feature at all since it denotes a so-called _polymorphic operation_ (invoked in this case on `Out`), which in MANOOL is deliberately the same
thing as a value of type Symbol (specified literally here), which can be applied to arguments as a procedure or any other procedure-like value.


Mechanisms and Techniques
----------------------------------------------------------------------------------------------------------------------------------------------------------------

### Background theory ##################################################################################################
[background theory]: #h:background-theory "Below: Background theory"

Together with the problem of programming language extensibility, let's consider also a broader problem of language modularity and composability (out of
sublanguages). First, here are some quick well-known facts from the programming language theory:

1. In order to be able to describe (in *finite* terms) the meaning of an *infinite* number of phrases (programs), the meaning of a phrase _&phi;_ is
   determined as some composition _M_ of meanings of its constituent phrases _&phi;_<sub>_i_</sub>
   with respect to some given set of phrase-formation operations:
       \[\[_&phi;_\]\] = _M_(\[\[_&phi;_<sub>0</sub>\]\], \[\[_&phi;_<sub>1</sub>\]\], ..., \[\[_&phi;_<sub>_n_-1</sub>\]\])[^c1]
     --- divide, and you'll master the infinity!

2. Thus, a phrase is either an elementary phrase (_p_<sub>_i_</sub>) or a composition of its constituent phrases _&phi;_<sub>_i_</sub> with respect to the
   phrase-formation operations mentioned above.

3. In case a phrase is a character string, the elementary phrases (_a_<sub>_i_</sub>) consist of individual characters, and the phrase-formation operation is
   just the string concatenation:
       _&alpha;_ = _&alpha;_<sub>0</sub>_&alpha;_<sub>1</sub>..._&alpha;_<sub>_n_-1</sub>.

4. In case a phrase is given in the form of a term (a tree) instead, the elementary phrases are nullary terms (_t_<sub>_i_</sub>), and the phrase-formation
   operations are term constructors:
       _&tau;_ = _f_(_&tau;_<sub>0</sub>, _&tau;_<sub>1</sub>, ..., _&tau;_<sub>_n_-1</sub>) for each non-nullary functor _f_.

5. The fundamental difference between the two phrase spaces (string and term) is that in contrast to term decomposition, string decomposition is *ambiguous*
   with respect to its phrase-formation operation alone (i.e., the concatenation operation applied in the process of phrase derivation).

6. For that reason, (syntactic) (de)composition of source code is often described by an (attributed) *unambiguous* context-free grammar, and the meaning of
   programs is ultimately described as the meaning of terms (i.e., _concrete or abstract syntax trees_) instead.[^c2]

7. The space of Lisp data (i.e., Lisp-style pairs plus atoms) constitutes a term algebra and is homomorphic to any other term algebra; thus, any claims about
   term algebras are automatically applicable to Lisp data and vice versa. The homomorphism is due to such mappings as

   >   _f_(_&tau;_<sub>0</sub>, _&tau;_<sub>1</sub>, ..., _&tau;_<sub>_n_-1</sub>) ---
   >   (_f_, _&tau;_<sub>0</sub>, _&tau;_<sub>1</sub>, ..., _&tau;_<sub>_n_-1</sub>) ---
   >   `(`_f_ _&tau;_<sub>0</sub> _&tau;_<sub>1</sub> ... _&tau;_<sub>_n_-1</sub>`)` =
   >   `(`_f_ `.` `(`_&tau;_<sub>0</sub> _&tau;_<sub>1</sub> ... _&tau;_<sub>_n_-1</sub>`)``)` ---
   >   (_f_, `(`_&tau;_<sub>0</sub> _&tau;_<sub>1</sub> ... _&tau;_<sub>_n_-1</sub>`)`) ---
   >   pair(_f_, `(`_&tau;_<sub>0</sub> _&tau;_<sub>1</sub> ... _&tau;_<sub>_n_-1</sub>`)`)

     Using Lisp data can be considered as *just* the most basic storage strategy for efficient term manipulation, and alternative data structures may be used as
   well.

8. Although some aspects of _AST_ (abstract syntax tree) validity and interpretation can be described themselves by (regular) tree grammars (which is a norm in
   MANOOL specification), such grammars are not really essential for the principle from the fact (1) to work (that is, for recursive definitions of meaning to
   do the job, term structure alone is already sufficient).

[^c1]: In practice, _M_, in turn, is a function of some context, such as a binding environment.

[^c2]: The theory of syntactic analysis for formal languages has been developed quite well long ago, and for simplicity, such grammars may be limited to only
       one, synthesized attribute and only term constructors within attribute equations.

Now, whereas some authors argue that arbitrary context-free grammars are inherently *anti-modular* and *non-composable* (as the language evolves it becomes
increasingly easy to introduce grammar ambiguities --- think C++) , others have studied grammar composability and proposed methods to adjust a union of two or
more grammars for the final grammar to make sense and be usable to describe syntax. However, according to its design philosophy, MANOOL takes an arguably
*simpler* route: for any two languages _L_ and _L'_ to be composable, they both should be defined by using *the same* context-free grammar (thus eliminating the
problem of grammar composability altogether).

### Universal syntax ###################################################################################################
[universal syntax]: #h:universal-syntax "Below: Universal syntax"

Ideally, the _universal syntax_ (as it *is* at any moment) suits any *future* needs in a programming language and therefore should change *very, very seldom*,
if at all. S-expressions (in any Lisp-family programming language) are described by such a syntax, and whereas MANOOL uses a slightly different notation, the
MANOOL syntax has the same dynamics as in Lisp(s).[^c3]

[^c3]: I had to modify the MANOOL syntax only once in four years since its inception (due to the introduction of the `\}...\{` notation for string literals).

A universal syntax is specified by a universal context-free grammar plus lexical rules (universal as well). Different attributed context-free grammars may
adequately do the job of syntactic decomposition for the same programming language, but for the idea of a universal grammar to work as well as possible, the
grammar should be as *general* as possible among those grammars. For instance,
  * instead of encoding _a_ `+` _b_ in the AST as add(_a_, _b_), it might be encoded as plus(_a_, _b_) or even `(+)`(_a_, _b_), and
  * instead of having a separate production for `if` ... `then` ... `else` ..., a universal production might be used to cover any construct in the form
    `{`_e_<sub>0</sub> _e_<sub>1</sub> ... _e_<sub>_n_-1</sub>`}`.

The grammar of MANOOL was formulated empirically (by trial and error) and is intended to cover approximately 80% of practical needs,[^c4] although given the
above design principles and the desire to restrict the basic character set to good-old ASCII, the design space was in fact narrow. As a result, for an untrained
eye the MANOOL notation (and my default rules of code indentation) may look unfamiliar or even weird; personally, I had a lot or resistance at first but got
used surprisingly quickly.[^c5] One of the sources of inspiration for me to use an unusual notation (if I had to at all) was [Smalltalk].

[^c4]: Yes, *only* 80%, or otherwise the syntax would become too complicated and overwhelmed with seldom used features (recall the *Pareto* principle!).

[^c5]: I also noticed that due to relatively high regularity, complex constructs in MANOOL do *not* look disproportionally uglier than simple ones, as it
       happens with some programming language notations originally designed for "readability" and "elegance".

Funnily enough, a universal syntax is not even mentioned among the MANOOL language design goals and has never been, but it has quickly become a part of the
implementation strategy; designing a new practical programming language requires a lot of experimentation, and thus it would be unproductive to have to deal
with context-free grammar changes every time a new feature is added, removed, or altered.

Compared to the "Hello, world!" example presented above, the following example uses
  * more library-supplied features (namely, `let rec ... in`, `proc ... as`, `if ... then ... else`, and `Out`),
  * more polymorphic operations (namely, `(==)`, `(*)`, `(-)`, and `WriteLine`), and
  * more _syntactic sugar_ (i.e., _syntactic features_):
^
    { {extern "manool.org.18/std/0.6/all"} in
    : let rec
      { Fact =
        { proc { N } as
        : if N == 0 then 1 else
          N * Fact[N - 1]
        }
      }
      in
      Out.WriteLine["Factorial of 10 is "; Fact[10]]
    }

This is a version of a classic "Hello, world!" analog for functional languages. I modified the original version to turn it into a complete, runnable program,
which writes to the standard output the factorial of 10 in decimal form by using a recursive definition of the factorial function.

Note that in its particular context above, `(=)` does *not* denote a polymorphic operation; it is just an "abstract operator" that serves to formulate
name-meaning associations syntactically (i.e., _a_ `=` _b_).

When translating the above code into an internal run-time representation, the MANOOL _syntactic analyzer_ (also known as the _parser_) first transforms the code
into an intermediate representation (that is, an AST), which incidentally, corresponds to the following equivalent code in MANOOL:

    { {extern "manool.org.18/std/0.6/all"} in
      { let rec
        { { (=) Fact
            { proc { N } as
              { if {(==) N 0} then 1 else
                {(*) N {Fact {(-) N 1}}}
              }
            }
          }
        }
        in
        {WriteLine Out "Factorial of 10 is " {Fact 10}}
      }
    }

  where any construct `{`_e_<sub>0</sub> _e_<sub>1</sub> ... _e_<sub>_n_-1</sub>`}` represents (in textual form) an internal AST node with _n_ successors
_e_<sub>0</sub>, _e_<sub>1</sub>, ..., _e_<sub>_n_-1</sub>, and almost everything else represents leaf nodes: `extern`, `"manool.org.18/..."`, `Fact`, `(=)`,
`(-)`, `1`, etc.

Note that instead of talking about tree nodes, we might talk about Lisp data (and consequently, about Lisp-style lists, pairs, and atoms) as well, but due to
some deep technical and philosophical reasons (which are beyond the scope of this article), we conform with just (unlabeled) internal nodes with _n_ successors
(or equivalently, _n_-element tuples or arrays) plus leaf nodes, which in practice gives us the same advantages. For the same reasons, the MANOOL core does
not support improper lists on the AST level and thus does not have any analog of (S-expression) dotted pair notation.[^c6]

[^c6]: There is at least one such precedent in the Lisp family of languages --- Clojure.

In somewhat general terms, the following identities hold in MANOOL on the AST level:

  * _a_`[`_b_<sub>0</sub>`;` _b_<sub>1</sub>`;` ...`;` _b_<sub>_n_-1</sub>`]` is equivalent to
    `{`_a_ _b_<sub>0</sub> _b_<sub>1</sub> ... _b_<sub>_n_-1</sub>`}` --- functional application notation (postfix-alike);
  * _a_`!` is equivalent to
    `(!)``[`_a_`]` --- postfix notation;
  * `~`_a_ is equivalent to
    `(~)``[`_a_`]` --- prefix notation;
  * _a_ `+` _b_ is equivalent to
    `(+)``[`_a_`;` _b_`]` --- left-associative infix notation (_a_ `+` _b_ `+` _c_ is equivalent to `(`_a_ `+` _b_`)` `+` _c_);
  * _a_`.`_b_`[`_c_<sub>0</sub>`;` _c_<sub>1</sub>`;` ...`;` _c_<sub>_n_-1</sub>`]` is equivalent to
    _b_`[`_a_`;` _c_<sub>0</sub>`;` _c_<sub>1</sub>`;` ...`;` _c_<sub>_n_-1</sub>`]`
    --- OOP<sup>ish</sup> notation (postfix-alike and left-associative infix-alike too);
  * _a_ `==` _b_ is equivalent to
    `(==)``[`_a_`;` _b_`]` --- non-associative infix notation;
  * `{`_a_<sub>0</sub> _a_<sub>1</sub> ... _a_<sub>_n_-1</sub>`:` _b_<sub>0</sub> _b_<sub>1</sub> ... _b_<sub>_m_-1</sub>`}` is equivalent to
    `{`_a_<sub>0</sub> _a_<sub>1</sub> ... _a_<sub>_n_-1</sub> `{`_b_<sub>0</sub> _b_<sub>1</sub> ... _b_<sub>_m_-1</sub>`}``}`
    --- right-associative cascading (MANOOL<sup>ish</sup>) notation; etc.

Note how applicants (i.e., _a_ in the first identity) and operators (e.g., `!`, `+`) correspond to leading (_head_) successors of internal AST nodes. This is
important because the head successor determines in a *uniform* way how the whole node is to be translated into an internal run-time representation (see
[below][core dispatcher]).

As you may note, the MANOOL syntax has syntactic sugar that acts *abstractly* and *independently* of what _a_`[`_b_<sub>0</sub>`;` _b_<sub>1</sub>`;` ...`;`
_b_<sub>_n_-1</sub>`]` and _a_ `+` _b_ actually mean. Some possible interpretations of MANOOL syntactic constructs are
  * an expression to be evaluated by using an applicative-order evaluation strategy,
  * the same but using a normal-order evaluation strategy,
  * an expression-like pattern to match tree-like (hierarchical) data,
  * a regular expression (in explicit tree-form) to match character strings,
  * a context-free grammar (or a production or a part thereof),
  * a [Prolog]-like (Horn) clause or a set thereof,
  * a structured query to a relational database,
  * an "elementary" function specified in algebraic (symbolic) form to undergo a symbolic differentiation,
  * a declarative description of an input-form layout along with a description of its dynamic behavior, or
  * just a part of a more complex construct (such as a name-meaning association _a_ `=` _b_ in a `let`-expression).

In all of the above cases, the MANOOL parser is to be reused, which makes up a foundation for a rich but *uniformly accessible* toolset and emphasizes the
relation of the above listed areas to abstract algebra(s), so to speak.

### The module system ##################################################################################################
[module system]: #h:the-module-system "Below: The module system"

In classic modular programming languages, such as [Modula-2] and [Ada], a module definition may serve for a *combination* of one or more purposes *at a time*:
  * to introduce name spaces,
  * to control name visibility (and thus to allow for an information hiding principle realization), and
  * to split programs across several source files (program units) and for separate compilation.

In MANOOL, instead, a number of *separate* and mutually *orthogonal* features are available that cover the above mechanisms and can be easily *combined*:
  * `export` is used to construct a module itself and thus introduces a name space;
  * when it is combined with `let`, name hiding becomes possible; and
  * `extern` allows you to reference a different program unit (including _foreign_ program units written in another programming language, for example, C++).

Let's replace in the first factorial example the usual `extern` expression with a module constructor `{... in: export ...}` that describes our own module
exporting only four features:

    { { {extern "manool.org.18/std/0.6/all"} in -- my local module
      : export let; proc; if; Out
      } -- end of local module
      in
    : let rec
      { Fact =
        { proc { N } as
        : if N == 0 then 1 else
          N * Fact[N - 1]
        }
      }
      in
      Out.WriteLine["Factorial of 10 is "; Fact[10]]
    }

This corresponds to the idea of _local modules_ in Modula-2 or _nested packages_ in Ada, but in this example the module does not even have a name and is
imported immediately instead of being bound to some name or made an external entity. This is useful for selective import of features and for information hiding
in the spirit of the `local ... end` construct of [Standard ML].

Now, to illustrate the idea of a fully-replaceable standard library proposed above, let's create two text files and place them in the same directory:

* `main.mnl`:

      { {extern "_my-lib.mnl"} in
      : let rec
        { Fact =
          { proc { N } as
          : if N == 0 then 1 else
            N * Fact[N - 1]
          }
        }
        in
        Out.WriteLine["Factorial of 10 is "; Fact[10]]
      }

* and `_my-lib.mnl`:

      {{extern "manool.org.18/std/0.6/all"} in: export let; proc; if; Out}

Although non-value entities are second-class entities, they are quite flexible otherwise --- they can be denoted by complex non-value expressions and bound to
names (aliased), statically (that is, at compile-time), for example:[^c7]

    { {{extern "manool.org.18/std/0.6/all"} in let}
      { myLib = {{extern "manool.org.18/std/0.6/all"} in: export let; proc; if; Out} } in
    : {myLib in let} { lambda = {myLib in proc} } in
    : {myLib in let} rec
      { Fact =
        { lambda { N } as
        : {myLib in if} N == 0 then 1 else
          N * Fact[N - 1]
        }
      }
      in
      {myLib in Out}.WriteLine["Factorial of 10 is "; Fact[10]]
    }

[^c7]: While this looks like a contrived example, it is intended to illustrate various possibilities.

The expression `{myLib in let}` denotes the feature `let` from the module `myLib` (also available as `let` from the standard library), and the `lambda` head
keyword in the expression `{lambda ... as ...}` behaves like `proc`.

Note that the keywords `in`, `as`, `rec`, `then`, and `else` above do *not* refer to "features", and therefore they cannot be aliased or turned (on their own,
separately and independently) into members of a module, unfortunately. Incidentally, this is one of the limitations of MANOOL I am talking about above; I argue,
however, that the MANOOL solution is still aligned with the 80/20 rule.

### Connection with S-expressions and Lisp(s) ##########################################################################

As you may note, under certain circumstances the notation of MANOOL closely resembles the notation of S-expressions (in any Lisp-family programming language)
where instead of parentheses (`()`) braces (`{}`) are used and on the other hand some symbols (namely, non-alphanumeric ones) appear enclosed in parentheses.
With "proper" indentation the S-expression equivalent of both factorial examples presented above looks like:

    ((extern "manool.org.18/std/0.6/all")
       in
       (let rec ((= Fact
                    (proc (N)
                      as
                      (if (== N 0)
                          then
                          1
                          else
                          (* N (Fact (- N 1)))))))
         in
         (WriteLine Out "Factorial of 10 is " (Fact 10))))

In contrast to the case of Lisp-family programming languages, here, in some cases elements (keywords) like `in`, `rec`, `as`, `then`, and `else`, are
*essential* for meaning and thus cannot be stripped off without introducing ambiguity. However, an equivalent program in Scheme has an almost identical
syntactic structure:

    (letrec ((Fact
              (lambda (N)
                (if (= N 0)
                    1
                    (* N (Fact (- N 1)))))))
      (display "Factorial of 10 is ")
      (display (Fact 10))
      (newline))

Some people love S-expressions as a programming language syntax, whereas others hate them, and people often have strong opinions and/or preferences in respect
of programming language notation, since it lies on the surface and stays in constant contact with language users. Thus, I suspect the former are going to
criticize me for not using the (concrete, surface) syntax of S-expressions for MANOOL, especially having such a good opportunity.[^c8] For now, just for now,
however, please regard the concrete syntax as a matter of personal choice that is irrelevant to the subject of this article. As I have promised, I'll publish a
separate article about the MANOOL syntax and its rationale.

[^c8]: This is not the first time that someone comes and suggests something to replace S-expressions and ... fails (beginning from never implemented
       _M-expressions_ from classic Lisp, which incidentally slightly resemble the notation of MANOOL). I do *not*, however, expect the same fate for MANOOL
       ;-).

### Homoiconicity, metaprogramming, syntactic macros ###################################################################
[homoiconicity]: #h:homoiconicity-metaprogramming-syntactic-macros "Below: Homoiconicity, metaprogramming, syntactic macros"

_Homoiconicity_ is the ability of a programming language to express programs that process other programs in a *very specific*, Lisp-like, way. Such languages
are called _homoiconic_, and hoiconicity has to do with _metaprogramming_ and _syntactic macros_. Homoiconicity, in particular, requires that ASTs could be
manipulated programmatically. Incidentally, MANOOL is a homoiconic language (and that is why its attributed context-free grammar has semantic evaluation
functions metacircularly specified as code in MANOOL).

This simple but a bit contrived example in MANOOL demonstrates that we can translate `if ... then` and `if ... then ... else` into, say, Spanish by defining a
syntactic macro (but making it available in a limited region only):

    { {extern "manool.org.18/std/0.6/all"} in
    : let
      { si =
        { macro
        : proc { F } as
        : if (Size[F] >= 7) & (F[2] == entonces') & (F[4] == si') & (F[5] == no') then
          {array of if# F[1] then' F[3] else'} + F[Range[6; Size[F]]] -- {si ... entonces ... si no ...}
          else
        : if (Size[F] >= 4) & (F[2] == entonces') then
          {array of if# F[1] then'} + F[Range[3; Size[F]]] -- {si ... entonces ...}
          else
          Nil -- fallback
        }
      }
      in -- you can use {si ...} hereinafter, up to the end of the compound expression, except where shadowed
    : let rec
      { Fact =
        { proc { N } as
        : si N == 0 entonces 1 si no -- hablo espanol
          N * Fact[N - 1]
        }
      }
      in
      Out.WriteLine["Factorial of 10 is "; Fact[10]]
    }

Don't worry if you don't understand this example fully right now --- just make sure that you can see for yourself from the code that this is possible (but stay
tuned if you are intrigued by this example and expect a future article with a detailed explanation of it ;-)

Translation Scheme
----------------------------------------------------------------------------------------------------------------------------------------------------------------

In this section I explain *how* it all works in MANOOL by citing relevant parts of the official specification (so, please bear with me).

This section presents the attributed context-free grammar and contains also quite a lot of advanced MANOOL code, but unfortunately, this article has *no* room
for detailed explanations. But don't be afraid --- this material is included merely for completeness and to better connect the ideas expressed in this article
with the MANOOL language reality --- you don't have to understand this material fully in order to appreciate these ideas (though, it may be helpful). Also
remember that you can always consult the MANOOL [Tutorial] and/or the official MANOOL [Specification] for more information.

[Tutorial]:      /tutorial/      "MANOOL Tutorial"
[Specification]: /specification/ "MANOOL Specification"

### Compilation phases #################################################################################################

So, according to the official MANOOL specification:

> To figure out the meaning of a form that makes up a program unit, the abstract machine transforms (compiles) the contents of the source file into an internal
> run-time representation, called _run-time code_.  
> ...
>
> A three-stage translation (i.e., compilation) scheme is suggested for the abstract machine:
>
> * _lexical analysis_ --- The input string of characters is split into lexical elements (lexemes), whose meaning is then encoded in left-to-right order as a
>   sequence of tokens. Note that in practice, whatever internal syntactic structure of individual lexemes is devised, it is generally unimportant for
>   determination of their meaning; rather, the lexical syntax is used for their sheer classification.
>
> * _syntactic analysis_ --- The string of terminal symbols that corresponds to the sequence of tokens resulting from the previous compilation phase undergoes a
>   syntactic analysis guided by a context-free grammar, which ultimately yields an _abstract syntax tree_ (AST) encoded as a MANOOL (semantic) value. Note that
>   in contrast to lexical analysis, here syntactic structure is essential for correct interpretation of source code.
>
> * _semantic analysis_ and _code generation_ --- The form and consistency (e.g., the presence and placement of certain keywords) of the AST resulting from the
>   previous compilation phase are checked, and finally, the run-time code is produced. Note that no new structural features are to be exposed on this stage, or
>   they would at least reflect closely those of the AST.

(See [The Abstract Machine](/specification/general#h:the-abstract-machine "MANOOL Specification: The Abstract Machine") and
[Translation Overview](/specification/general#h:translation-overview "MANOOL Specification: Translation Overview"))

### Grammar productions ################################################################################################

The context-free grammar of MANOOL is in fact quite simple (having a total of only 34 productions):

>     <start>S  ->  <datum>S
>     <datum>S  ->  <datum'>S
>     <datum>S  ->  <datum'>S[0] <equ op>L[1] <datum'>S[2]  [[ MakeList[{array of A[1]; A[0]; A[2]}] ]]
> ^
>     <datum'>S ->  <simple>S
>     <datum'>S ->  <simple>S[0] <rel op>L[1] <simple>S[2]  [[ MakeList[{array of A[1]; A[0]; A[2]}] ]]
> ^
>     <simple>S ->  <term>S
>     <simple>S ->  <simple>S[0] <add op>L[1] <term>S[2]  [[ MakeList[{array of A[1]; A[0]; A[2]}] ]]
> ^
>     <term>S   ->  <factor>S
>     <term>S   ->  <term>S[0] <mul op>L[1] <factor>S[2]  [[ MakeList[{array of A[1]; A[0]; A[2]}] ]]
> ^
>     <factor>S ->  <prim>S
>     <factor>S ->  <pref op>L[0] <factor>S[1]  [[ MakeList[{array of A[0]; A[1]}] ]]
> ^
>     <prim>S   ->  <prim'>S
>     <prim>S   ->  <prim>S[0] "["L <args>S[2] "]"L  [[ MakeList[{array of A[0]} + A[2]] ]]
>     <prim>S   ->  <prim>S[0] "."L <prim'>S[2] "["L <args>S[4] "]"L  [[ MakeList[{array of A[2]; A[0]} + A[4]] ]]
>     <prim>S   ->  <prim>S[0] <post op>L[1]  [[ MakeList[{array of A[1]; A[0]}] ]]
> ^
>     <prim'>S  ->  <lit>L[0]  [[ A[0] ]]  |  "("L <op>S[1] ")"L  [[ A[1] ]]
>     <prim'>S  ->  "{"L <list>S[1] "}"L  [[ A[1] ]]  |  "("L <datum>S[1] ")"L  [[ A[1] ]]
> ^
>     <op>S     ->  <equ op>L | <rel op>L | <add op>L | <mul op>L | <pref op>L | <post op>L
> ^
>     <args>S   ->  <args'>S  |  ""  [[ MakeList[{array}] ]]
> ^
>     <args'>S  ->  <datum>S[0] <args>S[1]  [[ MakeList[{array of A[0]} + A[1]] ]]
>     <args'>S  ->  <datum>S[0] ";"L <args'>S[2]  [[ MakeList[{array of A[0]} + A[2]] ]]
> ^
>     <list>S   ->  <list'>S  |  ""  [[ MakeList[{array}] ]]
> ^
>     <list'>S  ->  <datum>S[0] <list>S[1]  [[ MakeList[{array of A[0]} + A[1]] ]]
>     <list'>S  ->  <datum>S[0] ";"L <list'>S[2]  [[ MakeList[{array of A[0]} + A[2]] ]]
>     <list'>S  ->  <datum>S[0] ":"L <list'>S[2]  [[ MakeList[{array of A[0]; A[2]}] ]]
>
> where `MakeList` is some (pure) function, and for any array `A` of ASTs, the following condition is met:
>
>     MakeList[A].IsList[] & (Size[MakeList[A]] == Size[A]) & {for {E1 = MakeList[A]; E2 = A} all E1 == E2}

(See [The context-free grammar](/specification/core-language/syntax#h:the-context-free-grammar "MANOOL Specification: The context-free grammar")
for information on the notation and
[Lexical Structure](/specification/core-language/syntax#h:lexical-structure "MANOOL Specification: Lexical Structure")
for information on lexical elements and lexical categories)

### Compiler dispatcher ####################################################################################################
[core dispatcher]: #h:compiler-dispatcher "Below: Compiler dispatcher"

The official MANOOL specification has the following to say about the third compilation stage:

> Run-time code, which is the output of the semantic analysis and code generation translation phase, is represented by a _compiled entity_, which internally (in
> its turn) may contain other compiled entities. Incidentally, compiled entities are also what identifiers are bound to in a binding environment.
>
> A kind of object-oriented approach is used in this specification --- compiled entities are considered to be active agents (represented by a MANOOL
> value/object for the purposes of the metacircular description) capable of providing services whenever the abstract machine asks them to.
>
> The abstract machine has a (very) simple and compact semantic analysis and code generation _core dispatcher_. To compile an expression, the dispatcher first
> determines what kind of AST represents it:
>
> 1. If it is a symbol (explicitly) bound to a compiled entity in the active binding environment, that compiled entity becomes the result of compilation.
>
> 2. Otherwise, if it is an integer, a string, or a symbol, and the symbol starts with a character other than an ASCII lowercase letter (`a` thru `z`), the
>    abstract machine constructs a compiled entity that represents a literal value.
>
> 3. Otherwise, if it is a compound expression, the dispatcher compiles (recursively) the head subexpression and then asks the resulting compiled entity to
>    compile the whole expression.
>
> 4. Otherwise, if it is a special MANOOL value that encapsulates a compiled entity, which is produced by a `#`-expression (and for which the predicate `IsCode`
>    returns `True`), then the encapsulated compiled entity becomes the result of compilation.
>
> 5. As a fallback, the dispatcher reports an error as appropriate.
>
> Note that checking the presence and placement of keywords (such as `then`, `else`, `do`, etc.) is performed, if needed, by compiled entities rather than
> directly by the dispatcher. Also, a compiled entity, when asked to compile an expression, may in turn call the dispatcher for subexpressions and possibly
> specify a different active binding environment to compile in.

(See [Semantic Concepts](/specification/core-language/semantic-concepts#start "MANOOL Specification: Semantic Concepts") and
[Compiler Dispatcher](/specification/core-language/compiler-dispatcher#start "MANOOL Specification: Compiler Dispatcher"))

### Metacircular specification #########################################################################################

In this last subsection I present hypothetic code in MANOOL that serves a double purpose:
  * as a *formal*, _metacircular_ specification of the most important parts of the MANOOL semantics described above,[^d1] and
  * as an illustration of what developing in MANOOL *looks* and *feels* like (but leaving a few details without explanation).

[^d1]: Understanding a metacircular specification for a language _L_ requires prior knowledge of the language _L_, which raises a question about its utility.
       However, normally only *partial* knowledge is required, and this roughly corresponds to how children learn communicative systems (i.e., natural
       languages) from scratch.

> For the purposes of illustration, let's assume that a compiled entity recognizes the following polymorphic operations (beyond the standard ones):
>
> * `IsRvalue` --- tell whether the compiled entity corresponds to an r-value expression
>
> * `IsLvalue` --- tell whether the compiled entity corresponds to an l-value expression (which shall imply a positive answer to the above question as well)
>
> * `Compile` --- given an active binding environment represented by a mapping `SymTab`, compile the specified compound expression (whose head subexpression
>   corresponds to the compiled entity) to produce as a result another compiled entity
>
> * `Execute` --- given an evaluation context `Ctx`, evaluate the expression the compiled entity represents and produce a resulting value (applicable only in case
>   of r-value expressions)
>
> * `ExecIn` --- given an evaluation context `Ctx` and a value `Val`, store the value into the location the compiled entity represents (applicable only in case of
>   l-value expressions)
>
> * `ExecOut` --- given an evaluation context `Ctx`, move out the current value from the location the compiled entity represents (applicable only in case of
>   l-value expressions) to produce as a result the value moved out
>
> The core dispatcher algorithm described above may be specified more formally using the following metacircular description (in MANOOL):
>
>     { let rec
>       { Compile =
>         { proc { Form; SymTab } as
>         : if IsSym[Form] & SymTab.Exists[Form] then SymTab[Form] else -- bound identifier
>         : if IsInt[Form] | IsStr[Form] | IsSym[Form] &
>           { do True after
>           : if (Str[Form] <> "") & (Str[Form][0] >= "a"[0]$) & (Str[Form][0] <= "z"[0]$) then
>           : signal CompileError with "unbound keyword (nested in this context)"
>           }
>           then -- literal value
>           { let { _Form } in
>           : object { _Form = Form } with
>             IsRvalue = {proc {_} as True}
>             IsLvalue = {proc {_} as False}
>             Compile' = CompileApply
>             Execute  = {proc {Self; _} as Self[_Form]@}
>           }
>           else
>         : if IsList[Form] & (Size[Form] <> 0) then Compile[Form[0]; SymTab].(Compile')[Form; SymTab] else -- compound expression
>         : if IsCode[Form] then Form[_Entity]@ else -- a result of e# (used for metaprogramming)
>         : signal CompileError with "invalid form"
>         }
>       }
>       in
>     : export Compile
>     }
>
> As a matter of illustration, a compiled entity bound to the `if` keyword could be constructed by evaluating the expression
>
>     { object {} with
>       -- Classification
>       IsRvalue = {proc {_} as False}
>       IsLvalue = {proc {_} as False}
>       -- Compilation
>       Compile' =
>       { proc { _; Form; SymTab } as
>       : if (Size[Form] >= 6) & (Form[2] == then') & (Form[4] = else') then
>         { let { _Cond; _Body1; _Body2 } in
>         : object
>           { _Cond  = CompileRvalue[Form[1]; SymTab]
>             _Body1 = CompileRvalue[Form[3]; SymTab]
>             _Body2 = CompileRvalues[Form.Elems[Range[5; Size[Form]]]; SymTab]
>           }
>           with
>           -- Classification
>           IsRvalue = {proc {_} as True}
>           IsLvalue = {proc {Self} as Self[_Body1]@.IsLvalue[] & Self[_Body2]@.IsLvalue[]}
>           -- Execution
>           Execute = {proc {Self; Ctx} as Execute[Self[{if Execute[Self[_Cond]@; Ctx] then _Body1 else _Body2}]@; Ctx]}
>           ExecIn  = {proc {Self; Val; Ctx} as ExecIn[Self[{if Execute[Self[_Cond]@; Ctx] then _Body1 else _Body2}]@; Val; Ctx]}
>           ExecOut = {proc {Self; Ctx} as ExecOut[Self[{if Execute[Self[_Cond]@; Ctx] then _Body1 else _Body2}]@; Ctx]}
>           -- Compilation
>           Compile' = CompileApply
>         }
>         else
>       : if (Size[Form] >= 4) & (Form[2] == then') then
>         { let { _Cond; _Body } in
>         : object
>           { _Cond = CompileRvalue[Form[1]; SymTab]
>             _Body = CompileRvalues[Form.Elems[Range[3; Size[Form]]]; SymTab]
>           }
>           with
>           -- Classification
>           IsRvalue = {proc {_} as True}
>           IsLvalue = {proc {_} as False}
>           -- Execution
>           Execute = {proc {Self; Ctx} as: if Execute[Self[_Cond]@; Ctx] then Execute[Self[_Body]@; Ctx]}
>           -- Compilation
>           Compile' = CompileApply
>         }
>         else
>       : signal CompileError with "invalid form"
>       }
>     }

The above specification is very close to how expressions are evaluated in Lisp-family programming languages, although instead of evaluation we are talking here
about compilation.


Conclusions
----------------------------------------------------------------------------------------------------------------------------------------------------------------

Please make your conclusions yourself about whether the presented above approach and the limitations of MANOOL make sense.


[Iota and Jot]: //en.wikipedia.org/wiki/Iota_and_Jot                  "Wikipedia: Iota and Jot"
[Lisp]:         //en.wikipedia.org/wiki/Lisp_(programming_language)   "Wikipedia: Lisp"
[Common Lisp]:  //en.wikipedia.org/wiki/Common_Lisp                   "Wikipedia: Common Lisp"
[Scheme]:       //en.wikipedia.org/wiki/Scheme_(programming_language) "Wikipedia: Scheme"
[Clojure]:      //en.wikipedia.org/wiki/Clojure                       "Wikipedia: Clojure"
[Kernel]:       http://klisp.org                                      "klisp - a Kernel Programming Language implementation"
[Smalltalk]:    //en.wikipedia.org/wiki/Smalltalk                     "Wikipedia: Smalltalk"
[Forth]:        //en.wikipedia.org/wiki/Forth_(programming_language)  "Wikipedia: Forth"
[Tcl]:          //en.wikipedia.org/wiki/Tcl                           "Wikipedia: Tcl"
[Prolog]:       //en.wikipedia.org/wiki/Prolog                        "Wikipedia: Prolog"
[Modula-2]:     //en.wikipedia.org/wiki/Modula-2                      "Wikipedia: Modula-2"
[Ada]:          //en.wikipedia.org/wiki/Ada_(programming_language)    "Wikipedia: Ada"
[Standard ML]:  //en.wikipedia.org/wiki/Standard_ML                   "Wikipedia: Standard ML"

{%endraw%}{%include post_footer.md%}
