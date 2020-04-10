---
# specification/core-language/semantic-concepts.md
title:   Semantic Concepts &mdash; Specification
updated: 2019-12-21
---

<aside markdown="1" class="right">

See also:
  * [General](General#start)

</aside>

{%include spec_header.md%}{%raw%}


Memory Model
----------------------------------------------------------------------------------------------------------------------------------------------------------------

### Values, objects, special entities ##################################################################################

Conceptually, MANOOL programs operate with dynamically [typed][types] (and therefore _tagged_)[^1] values, which are _first-class_ citizens in the language ---
it is possible (at run-time and technically, also at compile-time) to
  * construct values dynamically (out of other, computed values, for example),
  * [assign](#) them to [variables],
  * pass as [arguments](#) to [procedures](#),
  * [return](#) as results of computations,

and so forth. Here are some examples of values:
  * [integer numbers](BasicDataTypes.html#h:integer "Integer") (such as `3`, `8`, or `10`),
  * [character strings](BasicDataTypes.html#h:string "String") (such as `"foo"` or `"Hello, world!"`),
  * procedures,
  * [input/output streams](#) (or rather references to them),
  * [dynamically managed variables](#) (or rather pointers to them).

[^1]: Thus, the integral value `3`, for example, with the tag (type) `Integer`, is distinct from the integral value "3" resulting from an evaluation of the
      expression `F64[3]` (which evaluates to a value of the type `Float64`).

Any [variable][variables] or [aggregate component](#) that holds some value requires physical _resources_ to account for that fact.[^2] Values in MANOOL are to
be understood merely as mathematical abstractions; in practice, values under the hood are rather represented and materialized as _objects_ --- first-class
entities that encapsulate resources.

[^2]: Resources are in many cases just memory blocks but are not restricted to that.

However, exactly which particular object represents a given value should be regarded as internal representation details of the (value/object) abstract data
type. In other words, the (object) [identity](#) between any two objects should be undetectable by (and mostly irrelevant for) MANOOL programs unless it is the
same as the (value) [equality](#) or [equivalence](#) between the same two objects. In this document we prefer to talk in terms of values as long as object
identity is irrelevant to the discourse and in terms of objects otherwise.

Note that:
  * Physical resources may be [shared](#) (fully or partially) among two or more variables or aggregate components that hold the same value (or even different
    values in case of partial sharing).
  * Completely different resources may represent the same value.
  * Different resources that represent the same value may be laid out in different structures (reflecting, for example, the history of their formation).

--- The intention of the MANOOL design is that exactly what resources are used should be normally irrelevant to what MANOOL programs compute; however, it may
still affect how fast they respond or how much memory they consume.

In MANOOL there is an important kind of (compile-time) entities, called _special entities_, which unlike objects are second-class citizens[^3] in the language
--- they may only be [bound](#) (similar to objects) to identifiers, by using [static](#) [bindings](#), and denoted at compile-time by non-value
[expressions](#). Entities made available from the MANOOL [standard library](#) as [`if`](#), [`while`](#), etc. as well as [modules](#) and [macros](#) are all
examples of special entities.

[^3]: In particular, second-class entities cannot be assigned to variables, passed as arguments to procedures, or returned as results of computations.

### Immutability/mutability of objects #################################################################################

Compare objects that represent integer numbers, character strings, input/output streams, and dynamically managed variables. The first two are examples of
_immutable_ objects. Conversely, the last two are examples of _mutable_ objects, whose externally observable _state_ can be altered in the course of program
execution and/or compilation.

Often, an immutable object models some abstract mathematical entity, whereas a mutable object models some real-world artifact (especially under object-oriented
programming paradigm). Note that irrespective of mutability, in MANOOL it is possible otherwise to treat both kinds of objects in the same way.

### Variables, evaluation contexts #####################################################################################

A _variable_ (or more accurately, a _temporary variable_) in MANOOL is a stateful (i.e., mutable) second-class entity that references some value (and therefore
some object), called the (current) _value of the variable_, at any given moment in the course of program execution and/or compilation. The current value of a
variable can be replaced by another value at any time (e.g., as a side effect resulting from an [assignment expression](#) evaluation), and more than one
variable at a time can reference the same object (whereby allowing for object and therefore resource _sharing_).

Since a temporary variable has a modifiable state, it resembles a mutable object. However, temporary variables (being second-class entities, unlike objects)
cannot be referred to dynamically --- a temporary variable may only be denoted by a [bound] identifier, statically (which is resolved in some sense at
compile-time)[^4].

[^4]: Variable bindings themselves are classified as [dynamic](#h:bindings-and-scopes-binding-environments), in spite of the qualifier "statically" used here.

An identifier can denote a temporary variable only indirectly, relative to an _evaluation context_, which will come into existence (at run-time only) upon each
[control-flow](#) entry into the corresponding [variable-binding expression](#) and cease its existence upon the matching exit (hence the name "temporary
variable"). Note that multiple evaluation processes may have been initiated and not yet completed for the same expression at the same time (due to a possibility
of [recursion](#) or even [concurrency or thread-level parallelism](#)) and that in this way multiple evaluation contexts for the same expression and
therefore multiple variables corresponding to the same variable binding may exist simultaneously.

Note that parameter identifiers that appear in the header of a [&lambda;-expression](#) denote in the body of that &lambda;-expression (except where shadowed)
regular temporary variables that provide [access](#) to the corresponding arguments (at run-time).

Also note that MANOOL has the concept of dynamic variables, which are different from ordinary (temporary) variables described here. For more information, see
[Dynamic Variables, pointers](#).

[bound]: #h:bindings-and-scopes-binding-environments "Bindings and scopes, binding environments"


### Object life-cycle and resource management ##########################################################################

Each object, whether mutable or not, may occupy _system resources_, such as virtual memory blocks, open file descriptors, or slots in a symbol table.

A value (or object) is said to be live (active) as long as there exists at least one variable that references it, either directly or as a container [§2.4.2]
element or component. Otherwise, it is said to be dead. Note that strictly speaking, according to these definitions, an object can sometimes be resurrected,
which does not, however, pose any issues or contradictions for the purposes of this document. The quality of liveness is not always definitive...

At any point in time, the abstract machine shall be able to reclaim and reuse the resources occupied by dead objects. For especially critical or scarce
resources, the abstract machine shall do it for each object once its lifetime becomes ended (becomes inactive).

### Data Types #########################################################################################################

In MANOOL the whole object (and equivalently, value) space is partitioned into disjoint classes called _data types_ (or, for short, _types_). If an object `x`
is said to be of the type `t`, then we write it by convention as `x:t`. Note that being of a specific type affects for an object its capability to be
(meaningfully) involved in specific computations.



[types]:     #h:data-types                    "Below: Data types"
[variables]: #h:variables-evaluation-contexts "Below: Variables, evaluation contexts"


Compilation and Evaluation Model
----------------------------------------------------------------------------------------------------------------------------------------------------------------

### Bindings and scopes, binding environments ##########################################################################

MANOOL is a block-structured programming language with static scoping --- each occurrence of an identifier in a program refers to a statically apparent binding
of that identifier. To _bind_ refers to an act of associating an identifier (represented by a [symbol]) with a meaning in a syntactically limited portion of
program code, called a _scope_. A _binding_ is a syntactic construct (which is a fragment of a [binding expression](#)) that represents such association.

Bindings are classified into _static bindings_ and _dynamic bindings_, and by default each symbol in a program denotes itself (or if you like, is bound to
itself) unless it starts with an ASCII lowercase letter (`a`, `b`, `c`, ... , `z`).

In case of a static binding, the meaning is either an object or a special entity. Static bindings may appear in [`let`-expressions](#).[^5] Using a module also
introduces static bindings into the scope, whereas an [`export`-expression](#) is used to record in a module specified static bindings for future reference.

[^5]: Note that [`let`-`rec`-expressions](#) cannot bind special entities.

In case of a dynamic binding, the identifier refers to a [temporary variable]. Dynamic bindings may appear in [`var`-forms](#), [`for`-forms](#), and as
parameters in parameter lists of [`proc`-forms](#).

The set of all effective bindings at some point in a program is known as the _binding environment_ in effect at that point. In block-structured programming
languages, a (new) binding may (temporarily) _shadow_ a currently effective binding (if any) for the same identifier in the scope of the new binding.

By convention, a symbol that denotes a special entity should not start with an ASCII uppercase letter (`A`, `B`, `C`, ... , `Z`) and should not start with a
lowercase letter (`a`, `b`, `c`, ... , `z`) otherwise. A symbol that starts with an underscore (`_`) should denote an [uninterned symbol][symbol].

[symbol]:             BasicDataTypes.html#h:symbol     "Symbol"
[temporary variable]: #h:variables-evaluation-contexts "Variables, evaluation contexts"

### Forms, expressions, control flow ###################################################################################

A _form_ in MANOOL is just a syntactic construct that is intended to be compiled (as a whole) into a run-time representation; the term "a form" may also refer
to an AST representation of such syntactic construct. When considered within a specific binding environment in which it is to be compiled, a form is referred to
as an _expression_, and the term may also refer to the corresponding run-time representation itself.

The notions of form and expression are context-dependent --- the same construct may or may not be considered a form or expression depending on its role in a
program or even the programmer's intent. For example, for a program unit consisting of the following expression:

    {{extern "manool.org:18/std/0.2/all"} in: proc {X; Y; Z} as X + Y + Z}

the following constituents are expressions:
  * `{extern "manool.org:18/std/0.2/all"}`,
  * `extern`,
  * `"manool.org.18/std/0.5/all"`,
  * `proc {X; Y; Z} as X + Y + Z`,
  * `proc`,

as well as `X + Y + Z`, `X + Y`, `X`, `Y`, `Z`, and both occurrences of `+`, whereas the following constructs (and their constituents, if any) are mere
fragments of forms: `in`, `{X; Y; Z}`, `as`.

Note that although the construct `{X; Y; Z}` by itself looks like a valid expression (which might be re-written also as `X[Y; Z]`), it is actually a fragment
due to its location and consequently, role.[^6]

[^6]: Also note that, for instance, `Y + Z` is not a syntactic construct at all; it is just an arbitrary source text fragment, due to left associativity of the
      operator `(+)`.

The classification of syntactic constructs into forms and form fragments reflects how the translator core of the abstract machine works, which is formally
specified [metacircularly](#). Forms, represented as ASTs, undergo directly a minimal analysis by the translator core, whereas fragments are not. The meaning of
each form can be determined only by considering the binding environment but otherwise independently of its placement in the code.

An _r-value expression_ is an expression that may occur on the right-hand side in an [assignment expression](#) and contains instructions for computation: an
r-value expression evaluates to a first-class value/object and may optionally engender computation side effects during its evaluation.

An _l-value expression_ is an expression that may occur on the left-hand side in an assignment expression: an l-value expression designates a virtual location
and provides instructions for storing an object into it. The currently stored object can be updated at any moment in the course of program compilation or
execution. Every l-value expression is also an r-value expression, and its evaluation yields the object currently stored in the virtual location. Thus, l-value
expressions may actually have different roles in MANOOL programs (either as an l-value or as an r-value expression).

The _control flow_ is said to enter or to exit an expression upon initiating or completing, respectively, of either of the following:
  * the expression evaluation,
  * updating a virtual location designated by that expression, or
  * moving out the current value of the virtual location designated by that expression.
  
Unlike an r-value expression, a _non-value expression_ is an expression that instead of evaluating to a first-class value/object resolves at compile-time to a
special entity (hence the term "non-value"), for example:

    if
    {{extern "manool.org.18/std/0.5/all"} in if}
    {macro: proc {F} as: if Size[F] <> 2 then {array} else F[1]#}

A form or expression that consists only of a literal or operator (when considering its AST representation) is called _primitive_ and _compound_ otherwise.

A compound expression whose first element is an r-value expression is called an _applicative expression_ and a _special expression_ otherwise.


{%endraw%}{%include spec_footer.md%}
