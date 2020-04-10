---
# specification/standard-library.md
title:   Standard Library &mdash; Specification
updated: 2019-11-7
---

{%include page_header.md%}{%raw%}

Metanotation
----------------------------------------------------------------------------------------------------------------------------------------------------------------

A combination of the following metalanguages and metalinguistic formalisms is used in the standard library specification:
  * plain English,
  * basic mathematical notation,
  * (regular) tree grammars,
  * MANOOL expressions with _syntactic placeholders_ (also known as _metalinguistic variables_ or _metavariables_, for short),
  * patterns matching MANOOL r-value expressions (namely, operation invocations), with _argument placeholders_ intended to match invocation arguments, augmented
    with data type annotations where it makes sense,
  * MANOOL expressions with argument placeholders (i.e., expression templates).

The standard library specification contains some introductory explanations in plain English followed by entries, each one describing a MANOOL feature or a group
of related features. An entry begins with either a tree grammar or an invocation pattern; a semantic description is then provided.

### Tree grammars ######################################################################################################

A tree grammar resembles a traditional formal grammar, but instead of describing a set of strings it describes a set of terms (i.e, trees). That is, in place of
string concatenation, term formation is used in the process of derivation. Tree grammars are always qualified as regular because they also resemble traditional
regular grammars due to similar fundamental properties they have (which is anyway irrelevant for our purposes).

The following is an example of a tree regular grammar (describing `if` special forms):

    <if form>  ->  {if <cond> then <body> else <alt body>[0] <alt body>[1] ... <alt body>[n-1]}
    <cond>     ->  <form>
    <body>     ->  <form>
    <alt body> ->  <form>

^

    <alt body>[0] <alt body>[1] ... <alt body>[n-1]

  means repetition 1 -- `n` times of items matching `<alt body>`, whereas

    <alt body>[0] ... <alt body>[n-1]

  would mean repetition just 0 -- `n` times.

### Expressions with syntactic placeholders ############################################################################

An example of an expression template with syntactic placeholders (used in the semantic specification for an `if` special form):

    {if <cond> then {do <body>[0]; ...; <body>[n-1]} else Nil}

^

    <body>[0] ... <body>[n-1]

  means repetition of all matches according to the corresponding tree grammar.

### Invocation patterns ################################################################################################

Examples of invocation patterns:

* The following is an invocation pattern with an argument placeholder `object` and a result type specified after `=>`:

      IsI48[object] => Boolean

* And the following is a similar invocation pattern where an argument placeholder `s` is followed by a type annotation `String` after `:`:

      I48[s:String] => Integer

* Sometimes concrete values may be specified instead of argument placeholders and result types:

      Order[Nil; Nil] => 0

* Dispatching arguments of polymorphic operations do not need type annotations; the type `Integer` is assumed here for the argument placeholder `x`:

      x + y:Integer => Integer

* Results may be sometimes named to reinforce clarity:

      ~x => minus-x:Integer

### Expression templates with argument placeholders ####################################################################

Example of an expression template with argument placeholders `x` and `y` (from the specification of `Rem` for `Integer`):

    {unless 0 <> y signal Undefined else x - x / y * y}


<!--An invocation template looks just like a normal operation invocation augmented with metalinguistic _placeholders_ (i.e., variables that represent invocation
arguments) and a result data type --- separated by `=>` --- plus a result name when it helps clarity. Names of metalinguistic variables are all lowercase and
are followed by a datatype after a `:` character (except for dispatch control parameters where the data type is implied and except for type-unrestricted
parameters). The result name (if present) precedes the result type and a `:` character. For each invocation template a semantic description is provided.-->

Contents
----------------------------------------------------------------------------------------------------------------------------------------------------------------

* [Basic Data Types](BasicDataTypes.html)

{%endraw%}{%include page_footer.md%}
