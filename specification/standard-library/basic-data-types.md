---
# specification/standard-library/basic-data-types.md
title:   Basic Data Types &mdash; Specification
updated: 2019-11-05
---

{%include spec_header.md%}{%raw%}

This section describes the most fundamental non-composite data types provided by the MANOOL standard library.

Metanotation
----------------------------------------------------------------------------------------------------------------------------------------------------------------

The following metalinguistic formalisms and metalanguages are used in this section:
  * plain English,
  * basic mathematical notation,
  * tree regular grammars,
  * simple patterns and templates for MANOOL r-value expressions (namely, operation invocations) with pattern/template variables (also known as
    _metavariables_).

An invocation template looks just like a normal operation invocation augmented with metalinguistic _placeholders_ (i.e., variables that represent invocation
arguments) and a result data type --- separated by `=>` --- plus a result name when it helps clarity. Names of metalinguistic variables are all lowercase and
are followed by a datatype after a `:` character (except for dispatch control parameters where the data type is implied and except for type-unrestricted
parameters). The result name (if present) precedes the result type and a `:` character. For each invocation template a semantic description is provided.


Integer
----------------------------------------------------------------------------------------------------------------------------------------------------------------

The type `Integer` corresponds to the (finite) set of integral numbers in the range -(2<sup>47</sup>-1) thru +(2<sup>47</sup>-1) (i.e., -140737488355327 thru
+140737488355327), inclusive.[^1] Objects of type `Integer` are often referred to simply as _integers_, and like most abstract mathematical entities, integers
are [immutable]. Non-negative integers can have [literal representation] in MANOOL programs.

[^1]: The value -2<sup>47</sup>, typical for two's complement binary representations, is excluded from this range.

### Constructors #######################################################################################################

  ******************************************************************************
    I48[s:String] => Integer

  --- evaluates to an integer that represents the value specified by the argument in the conventional[^2]

  * decimal notation:

        ("+" | "-" | ) <nonzero digit> (<nonzero digit> | "0")*

  * hexadecimal notation:

        ("+" | "-" | ) "0" ("x" | "X") <hex digit> <hex digit>*

  * or octal notation:

        ("+" | "-" | ) "0" <octal digit> <octal digit>*

  where

    <nonzero digit> ->  "1" | "2" | "3" | ... | "9"
    <hex digit>     ->  "0" | "1" | "2" | ... | "9" | "A" | "B" | "C" | ... | "F" | "a" | "b" | "c" | ... | "f"
    <octal digit>   ->  "0" | "1" | "2" | ... | "7"

  ******************************************************************************
    I48[i:Integer] => just-i:Integer

  --- evaluates to the argument itself

[^2]: This notation is also stipulated by [C] and [POSIX] specifications.

### Type predicates ############################################################
    IsI48[object] => Boolean

### Polymorphic operations #############################################################################################

  ******************************************************************************
    x + y:Integer => Integer

  --- addition

  ******************************************************************************
    x - y:Integer => Integer

  --- subtraction

  ******************************************************************************
    x * y:Integer => Integer

  --- multiplication

  ******************************************************************************
    x / y:Integer => Integer

  --- integer division --- the fractional part of the real result is truncated, e.g.:

    (8 / 3 == 2) & (8 / ~3 == ~2)

  ******************************************************************************
    x.Rem[y:Integer] => Integer

  --- remainder of `(/)` --- equivalent to

    {unless 0 <> y signal Undefined else x - x / y * y}

  ******************************************************************************
    x.Div[y:Integer] => Integer

  --- floored division --- the real result is floored, e.g.:

    (8.Div[3] == 2) & (8.Div[~3] == ~3)

  ******************************************************************************
    x.Mod[y:Integer] => Integer

  --- modulo (i.e., remainder of `Div`) --- equivalent to

    {unless 0 <> y signal Undefined else x - x.Div[y] * y}

  ******************************************************************************
    Neg[x] => Integer, ~x => minus-x:Integer

  --- negation (unary minus)

  ******************************************************************************
    x == y => Boolean

  --- comparison for equality

  ******************************************************************************
    x <> y => Boolean

  --- comparison for inequality

  ******************************************************************************
    x < y => Boolean

  --- comparison for "less than"

  ******************************************************************************
    x <= y => Boolean

  --- comparison for "less than or equal"

  ******************************************************************************
    x > y => Boolean

  --- comparison for "greater than"

  ******************************************************************************
    x >= y => Boolean

  --- comparison for "greater than or equal"

  ******************************************************************************
    Order[x; y:Integer] => Integer

  --- total order/equivalence relation --- equivalent to

    {if x < y then ~1 else: if x > y then 1 else 0}

  ******************************************************************************
    Abs[x] => Integer

  --- absolute value (magnitude, modulus)

  ******************************************************************************
    Str[x] => String

  --- decimal representation, e.g.:

    (Str[10] == "10") & (Str[~3] == "-3")

  ******************************************************************************
    Str[x; format:String] => String

  --- argument string representation formatted according to a [C]/[POSIX] `printf` [format specifier] for the `int` type, with the leading `%` character
  stripped, e.g.:

    (Str[10; "3d"] == " 10") & (Str[10; "+03d"] == "+010")

  ******************************************************************************
    Clone[x] => just-x:Integer, DeepClone[x] => just-x:Integer

  --- evaluates to the argument itself

### Exceptions #################################################################
* `Overflow` --- no object of type `Integer` can represent the mathematical result of the operation (e.g., when calculating the product of 2147483648 by
  2147483648)
* `DivisionByZero` --- attempting to divide a dividend different from zero by zero
* `Undefined` --- attempting to divide zero by zero
* `SyntaxError` --- a string representation or format specifier is malformed (has invalid syntax)
* `UnrecognizedOperation`
* `InvalidInvocation`
* `TypeMismatch`
* `HeapExhausted`
* `StackOverflow`
* `LimitExceeded`


String
----------------------------------------------------------------------------------------------------------------------------------------------------------------

The type `String` corresponds to the (infinite) set of (finite) sequences of 0 or more elements (octets)[^3] drawn from a (finite) alphabet of 2<sup>8</sup>
(i.e., 256) elements. Objects of type `String` are often referred to simply as _strings_ (or more accurately, _raw strings_), and like most abstract
mathematical entities, strings are [immutable]. Strings can have [literal representation] in MANOOL programs in most practically interesting cases.

[^3]: In practice, however, there will be always some dynamic limit imposed on the maximum size of strings representable in computer memory at any given moment.

Typically, strings are not directly interpreted in accordance with the above definition. Instead, raw strings will contain actual character strings encoded
using some specific character encoding, like ASCII, UTF-8, ISO-8859-1, or even UCS-32. However, by themselves and when considered outside of any additional
context, raw strings are otherwise character-encoding agnostic and even might be used to manipulate non-character data, such as the contents of arbitrary binary
files.

String octets are indexed starting from zero and are accessible as [Unsigned](#h:unsigned "Below: Unsigned") values from `U32[0]` to `U32["0xFF"]`, inclusive.

### Constructors #######################################################################################################

  ******************************************************************************
    S8[s:String] => just-s:String

  --- evaluates to the argument itself (this constructor is provided merely for completeness)

### Named constants ####################################################################################################
    Nul == "" | U32[ 0]
    Esc == "" | U32[27]
    Bel == "" | U32[ 7]
    BS  == "" | U32[ 8]
    HT  == "" | U32[ 9]
    LF  == "" | U32[10]
    VT  == "" | U32[11]
    FF  == "" | U32[12]
    CR  == "" | U32[13]
    Sp  == "" | U32[32] -- equivalent to " "

### Type predicates ############################################################
    IsS8[object] => Boolean

### Polymorphic operations #############################################################################################

  ******************************************************************************
    s[index:Integer] => octet:Unsigned

  --- octet at the specified position

  ******************************************************************************
    s[indexes:(Rev)Range] => substring:String

  --- substring of octets in the specified range of positions

  ******************************************************************************
    s.Repl[index:Integer; octet:Unsigned] => mod-s:String

  --- evaluates to a string that represents the original string value with the specified octet replaced by the specified new value

  ******************************************************************************
    s.Repl[indexes:(Rev)Range; substring:String] => mod-s:String

  --- equivalent to

    s[Range[Lo[indexes]] + substring + s[Range[Hi[indexes]; Size[s]]

  ******************************************************************************
    Size[s] => Integer

  --- string size (length, number of elements)

  ******************************************************************************
    x + y => concat:String

  --- concatenation

  ******************************************************************************
    s | octet:Unsigned => concat:String

  --- concatenation with a single element

  ******************************************************************************
    Elems[s] => just-s:String

  --- evaluates to the argument itself

  ******************************************************************************
    s.Elems[indexes:(Rev)Range] => slice:Iterator

  --- evaluates to a slice iterator that represents the lazily evaluated substring of octets in the specified range of positions

  ******************************************************************************
    Keys[s] => indexes:Range

  --- evaluates to a (forward) range that represents element indexes

  ******************************************************************************
    s.Keys[indexes:(Rev)Range] => just-indexes:(Rev)Range

  --- evaluates to the specified range of element indexes

  ******************************************************************************
    s^ => just-s:String

  --- evaluates to the argument itself

  ******************************************************************************
    x == y => Boolean

  --- comparison for equality

  ******************************************************************************
    x <> y => Boolean

  --- comparison for inequality

  ******************************************************************************
    Order[x; y] => Integer

  --- total order/equivalence relation --- `~1` iff `x` lexicographically precedes `y`, `1` iff `y` lexicographically precedes `x`, and `0` otherwise

  ******************************************************************************
    Str[s] => just-s:String

  evaluates to the argument itself

  ******************************************************************************
    Str[s; format:String] => String

  --- argument string representation formatted according to a [C]/[POSIX] `printf` [format specifier] for the `const char *` type, with the leading `%`
  character stripped, e.g.:

    Str["Hi"; "3s"] == " Hi", Str["Hi"; "-3s"] == "Hi "

  ******************************************************************************
    Clone[s] => String, DeepClone[s] => String

  --- evaluates to an (initially) unshared string that represents the same value as the argument

### Exceptions #################################################################
* `IndexOutOfRange`
* `ConstraintViolation`
* `UnrecognizedOperation`
* `InvalidInvocation`
* `TypeMismatch`
* `LimitExceeded`
* `HeapExhausted`
* `StackOverflow`


Symbol
----------------------------------------------------------------------------------------------------------------------------------------------------------------

Objects of type `Symbol` are often referred to simply as _symbols_. Symbols resemble strings --- they can represent arbitrary sequences of octets (with very few
exceptions). However, symbols are different from strings in either the set of basic operations the MANOOL standard library provides for these types, their exact
behavior, or performance characteristics of some operations. Symbol is one of the fundamental data types in MANOOL (like [Integer](#h:integer "Above: Integer")
and [String](#h:string "Above: String")).

Compared to general-purpose strings, symbols are intended to identify other entities in a given context by using symbolic names (except uninterned symbols), and
their internal representation shall be optimized for this purpose.[^4] In particular, symbols shall be comparable in constant time and also shall be able to be
used as keys in certain lookup operations with constant-time complexity (so-called symbol-table lookup operations). The cost of such advantage may be paid at
the moment of construction of a symbol, which may take longer than for strings.

[^4]: There may be imposed some implementation-defined limit on the total number of alive symbols that can exist at any given moment in the course of program
      execution and/or compilation.

There are two kinds of symbols: _interned symbols_ and _uninterned symbols_. The former can be obtained by conversion from strings, whereas the later
cannot --- they can only be generated (by invocations of `MakeSym`):

### Constructors #######################################################################################################

  ******************************************************************************
    <quote-form> -> <datum>'

  --- see [Metaprogramming](MetaProg.html)

  ******************************************************************************
    MakeSym[s:String] => Symbol

  --- unless the argument starts with <code>`</code>, evaluates to an interned symbol that represents the same sequence of octets as the argument

  ******************************************************************************
    MakeSym[] => Symbol

  --- evaluates to an (randomly generated) uninterned symbol that is not alive at the moment of the evaluation

### Named constants ####################################################################################################
In MANOOL programs a symbol (name) that has not been explicitly bound denotes itself unless it starts with an ASCII lowercase letter. Note that in neither case
symbol construction is completely referentially transparent.

### Type predicates ############################################################
    IsSym[object] => Boolean

### Polymorphic operations #############################################################################################

  ******************************************************************************
    x == y => Boolean

  --- comparison for equality

  ******************************************************************************
    x <> y => Boolean

  --- comparison for inequality

  ******************************************************************************
    Order[x; y:Symbol] => Integer

  --- total order/equivalence relation (once a symbol is obtained for the first time in the course of program execution or compilation or resurrected, its order
  with respect to other alive symbols is established in an implementation-defined way)

  ******************************************************************************
    op[...]

  --- polymorphic operation on the arguments

  ******************************************************************************
    Str[s] => String

  --- string representation (e.g., `Str[Foo'] == "Foo"`, whereas `Str[MakeSym[]]` returns something random like <code>"`25120"</code>)
  ******************************************************************************
    Clone[s] => just-s:Symbol, DeepClone[s] => just-s:Symbol

  evaluates to the argument itself

### Exceptions #################################################################
* `ConstraintViolation`
* `UnrecognizedOperation`
* `InvalidInvocation`
* `TypeMismatch`
* `HeapExhausted`
* `StackOverflow`
* `LimitExceeded`


Boolean
----------------------------------------------------------------------------------------------------------------------------------------------------------------

There are only two objects of type `Boolean`---these are truth values _true_ and _false_, which are bound to the names `True` and `False` in the MANOOL standard
library, respectively.

### Named constants ############################################################
    True
    False

### Type predicates ############################################################
    IsBool[object] => Boolean

### Polymorphic operations ############################################################################################

  ******************************************************************************
    x & y:Boolean => Boolean

  --- conjunction (logical "and")

  ******************************************************************************
    x | y:Boolean => Boolean

  --- disjunction (logical "or")

  ******************************************************************************
    ~x => Boolean

  --- negation (logical "not")

  ******************************************************************************
    x.Xor[y:Boolean] => Boolean

  --- logical exclusive "or"

  ******************************************************************************
    x == y => Boolean

  --- comparison for equality

  ******************************************************************************
    x <> y => Boolean

  --- comparison for inequality

  ******************************************************************************
    Order[x; y:Boolean] => Integer

  --- total order/equivalence relation:

    Order[False; False] == Order[True; True] == 0, Order[False; True] == ~1, Order[True; False] == 1

  ******************************************************************************
    Str[x] => String

  --- representation as a string:

    Str[True] == "True", Str[False] == "False"

  ******************************************************************************
    Clone[x] => just-x:Boolean, DeepClone[x] => just-x:Boolean

  --- evaluates to the argument itself

### Exceptions #################################################################
* `UnrecognizedOperation`
* `InvalidInvocation`
* `TypeMismatch`
* `HeapExhausted`
* `StackOverflow`
* `LimitExceeded`


Null
----------------------------------------------------------------------------------------------------------------------------------------------------------------

There is just one object of type `Null` --- _nil_, which is bound to the name `Nil` in the MANOOL standard library.

### Named constants ############################################################
    Nil

### Type predicates ############################################################
    IsNull[object] => Boolean

  --- provided merely for completeness --- equivalent to `Nil == object`

### Polymorphic operations #############################################################################################

  ******************************************************************************
    Nil == y => Boolean

  --- comparison for equality to `Nil`

  ******************************************************************************
    Nil <> y => Boolean

  --- comparison for inequality to `Nil`

  ******************************************************************************
    Nil^, Set[Nil; object]

  --- `IndirectionByNil` is raised

  ******************************************************************************
    Order[Nil; Nil] => 0

  --- total order/equivalence relation

  ******************************************************************************
    Str[Nil] => "Nil"

  --- representation as a string

  ******************************************************************************
    Clone[Nil] => Nil, DeepClone[Nil] => Nil

  --- evaluates to the argument itself

### Exceptions #################################################################
* `IndirectionByNil`
* `UnrecognizedOperation`
* `InvalidInvocation`
* `TypeMismatch`
* `HeapExhausted`
* `StackOverflow`
* `LimitExceeded`


Floating-Point
----------------------------------------------------------------------------------------------------------------------------------------------------------------

The type Floating-Point (or more accurately, Binary Floating-Point) corresponds to a (finite) set of real numbers and special values representable according to
the IEEE 754-2008 standard[^5] in a base-2 format, including all normal and subnormal numbers and (positive and negative) zeros and excluding not-a-numbers
(NaNs) and (positive and negative) infinities. Like most abstract mathematical entities, objects of type Binary Floating-Point are [immutable].

[^5]: ... also designated as ISO/IEC/IEEE 60559:2011 and as ANSI/IEEE 754-2008 ...

All basic operations on Binary Floating-Point values provided by the MANOOL standard library shall be performed in conformance with IEEE 754-2008 unless
explicitly required otherwise. The effective rounding mode for all calculations shall be always "round to nearest, ties to even" (which has no relation to the
rounding performed by such operations as `Trunc` and `Round`).

### Constructors #######################################################################################################

There are actually two disjoint Binary Floating-Point types (one for each of the two common base-2 formats specified in IEEE 754-2008): 32-bit
(single-precision) Floating-Point and 64-bit (double-precision) Floating-Point. In the following chart these types are denoted as `Float32` and `Float64`,
respectively:

  *****************************************************************************
    F64[s:String] => Float64
    F32[s:String] => Float32

  --- evaluates to a 64-bit or 32-bit Floating-Point object, respectively, that represents the value specified by the argument in the conventional decimal
  notation:

    ("+" | "-" | ) (<digit> <digit>* ("." <digit>* | ) | "." <digit> <digit>*) (("e" | "E") ("+" | "-" | ) <digit> <digit>* | )

  where

    <digit> ->  "0" | "1" | "2" | ... | "9"

  *****************************************************************************
    F64[i:Integer] => Float64
    F32[i:Integer] => Float32

  --- equivalent to `F64[Str[i]]` or `F32[Str[i]]`, respectively

### Type predicates ############################################################
    IsF64[object] => Boolean
    IsF32[object] => Boolean

### Polymorphic operations #############################################################################################

In the following chart `Float` refers to the underlying Binary Floating-Point type, and `t` stands for the corresponding constructor (i.e., either `F32` or
`F64`):

  *****************************************************************************
    x + y:Float => Float

  --- addition

  *****************************************************************************
    x - y:Float => Float

  --- subtraction

  *****************************************************************************
    x * y:Float => Float

  --- multiplication

  *****************************************************************************
    x / y:Float => Float

  --- division

  *****************************************************************************
    x.Rem[y:Float] => Float

  --- remainder of integer division, e.g.:

    F64["9.5"].Rem[F64["3.5"]] == F64["2.5"]

  *****************************************************************************
    Neg[x] => Float, ~x => minus-x:Float

  --- negation (unary minus)

  *****************************************************************************
    x.Fma[y:Float; z:Float] => Float

  --- fused multiply-add operation --- computes `y`*`z`+`x` avoiding undue loss of precision

  *****************************************************************************
    x == y => Boolean

  --- comparison for equality

  *****************************************************************************
    x <> y => Boolean

  --- comparison for inequality

  *****************************************************************************
    x < y:Float => Boolean

  --- comparison for "less than"

  *****************************************************************************
    x <= y:Float => Boolean

  --- comparison for "less than or equal"

  *****************************************************************************
    x > y:Float => Boolean

  --- comparison for "greater than"

  *****************************************************************************
    x >= y:Float => Boolean

  --- comparison for "greater than or equal"

  *****************************************************************************
    Order[x; y:Float] => Integer

  --- total order/equivalence relation --- `~1` iff `x` is ordered before `y`, `1` iff `y` is ordered before `x`, and `0` otherwise

  *****************************************************************************
    Abs[x] => Float

  --- absolute value (magnitude, modulus)

  *****************************************************************************
    Sign[x] => Float

  --- sign function --- for non-zero arguments evaluates to `t[1]`/`~t[1]` iff the argument is positive/negative, respectively; otherwise, evaluates to the
  argument itself (i.e., either `t[0]` or `~t[0]`)

  *****************************************************************************
    Sign[x; y:Float] => copysign:Float

  --- evaluates to a value with the magnitude of `x` and the sign of `y`

  *****************************************************************************
    Exp[x] => Float

  --- base-e exponential of the argument

  *****************************************************************************
    Expm1[x] => Float

  --- computes the base-e exponential of the argument, minus 1, with a greater accuracy than `Exp[x] - t[1]` does

  *****************************************************************************
    Log[x] => Float

  --- base-e (natural) logarithm of the argument

  *****************************************************************************
    Log1p[x] => Float

  --- computes the base-e (natural) logarithm of 1 plus the argument with a greater accuracy than `Log[t[1] + x]` does

  *****************************************************************************
    Log[base; x:Float] => Float

  --- equivalent to `Log[x] / Log[base]`

  *****************************************************************************
    Log10[x] => Float

  --- computes the base-10 logarithm of the argument (with a greater accuracy than `Log[t[10]; x]` does)

  *****************************************************************************
    Log2[x] => Float

  --- computes the base-2 logarithm of the argument (with a greater accuracy than `Log[t[2]; x]` does)

  *****************************************************************************
    Sqr[x] => Float

  --- square (`x`<sup>2</sup>, `x`*`x`)

  *****************************************************************************
    Sqrt[x] => Float

  --- square root of the argument

  *****************************************************************************
    Hypot[x; y:Float] => Float

  --- computes the square root of the sum of the squares of both arguments avoiding undue overflow

  *****************************************************************************
    Cbrt[x] => Float

  --- cube root of the argument

  *****************************************************************************
    x.Pow[y:Float] => Float

  --- computes `x` raised to the power `y`

  *****************************************************************************
    Sin[x] => Float

  --- sine of an angle in radians

  *****************************************************************************
    Cos[x] => Float

  --- cosine of an angle in radians

  *****************************************************************************
    Tan[x] => Float

  --- tangent of an angle in radians

  *****************************************************************************
    Asin[x] => Float

  --- arcsine in radians

  *****************************************************************************
    Acos[x] => Float

  --- arccosine in radians

  *****************************************************************************
    Atan[x] => Float

  --- arctangent in radians

  *****************************************************************************
    Atan[y; x:Float] => atan2:Float

   --- computes the value of the arctangent of `y`/`x` using the signs of both arguments to determine the quadrant of the result

  *****************************************************************************
    Sinh[x] => Float

  --- hyperbolic sine

  *****************************************************************************
    Cosh[x] => Float

  --- hyperbolic cosine

  *****************************************************************************
    Tanh[x] => Float

  --- hyperbolic tangent

  *****************************************************************************
    Asinh[x] => Float

  --- inverse hyperbolic sine

  *****************************************************************************
    Acosh[x] => Float

  --- inverse hyperbolic cosine

  *****************************************************************************
    Atanh[x] => Float

  --- inverse hyperbolic tangent

  *****************************************************************************
    Erf[x] => Float

  --- error function of the argument

  *****************************************************************************
    Erfc[x] => Float

  --- complementary error function of the argument (equivalent to: `t[1] - Erf[x]`)

  *****************************************************************************
    Gamma[x] => Float

  --- gamma function of the argument

  *****************************************************************************
    Lgamma[x] => Float

  --- computes the natural logarithm of the gamma function of the argument avoiding undue overflow

  *****************************************************************************
    Jn[x; n:Integer] => Float

  --- Bessel function of the first kind of order `n` of `x`

  *****************************************************************************
    Yn[x; n:Integer] => Float

  --- Bessel function of the second kind of order `n` of `x`

  *****************************************************************************
    Trunc[x] => Float

  --- evaluates to the argument with the fractional part discarded

  *****************************************************************************
    Round[x] => Float

  --- evaluates to the argument rounded to the nearest integral value, half ties toward infinity, e.g.:

    (Round[t[".5"]] == t[1]) & (Round[~t[".5"]] == ~t[1])

  *****************************************************************************
    Floor[x] => Float

  --- evaluates to the nearest integral value less than or equal to the argument

  *****************************************************************************
    Ceil[x] => Float

  --- evaluates to the nearest integral value greater than or equal to the argument

  *****************************************************************************
    Int[x] => Integer

  --- `Trunc[x]` represented as an object of type `Integer`

  *****************************************************************************
    Str[x] => String

  --- decimal representation, e.g.:

    Str[F64["10.5"]] == "1.0500000000000000e+01"

  ******************************************************************************
    Str[x; format:String] => String

  --- argument string representation formatted according to a [C]/[POSIX] `printf` [format specifier] for the `double` type, with the leading `%` character
  stripped, e.g.:

    (F64["10.5"].Str["6.2f"] == " 10.50") & (F64["10.5"].Str["9.2E"] == " 1.05E+01")

  ******************************************************************************
    Clone[x] => just-x:Float, DeepClone[x] => just-x:Float

  --- evaluates to the argument itself

### Exceptions #################################################################
* `Overflow` --- the rounded mathematical result of the operation does not fit into the destination floating-point format (e.g., when evaluating
  `Exp[F64[1000]]`)
* `DivisionByZero` --- the mathematical result of the operation is undefined and the function or operation has a pole for the exact value of the argument (e.g.,
  when evaluating `F64[1] / F64[0]` or `Log[F64[0]]`)
* `Undefined` --- in any other case when the mathematical result of the operation is undefined (e.g., when evaluating `F64[0] / F64[0]`, `Sqrt[~F64[1]]`, or
  `Log[~F64[1]]`)
* `SyntaxError` --- a string representation or format specifier is malformed (has invalid syntax)
* `UnrecognizedOperation`
* `InvalidInvocation`
* `TypeMismatch`
* `HeapExhausted`
* `StackOverflow`
* `LimitExceeded`


Decimal Floating-Point
----------------------------------------------------------------------------------------------------------------------------------------------------------------

The type Decimal Floating-Point corresponds to the (finite) set of floating-point values representable according to the [IEEE 754-2008] standard in a base-10
format excluding negative zero(s), not-a-numbers (NaNs), and (positive and negative) infinities. Like most abstract mathematical entities, objects of type
Decimal Floating-Point are immutable.

All basic operations on Decimal Floating-Point values provided by the MANOOL standard library shall be performed in conformance with IEEE 754-2008 unless
explicitly required otherwise.

### Constructors #######################################################################################################

There are actually four disjoint Decimal Floating-Point types, one for each combination of precision (64-bit or 128-bit) and rounding mode for calculations
("round to nearest, ties away from zero" or "round to nearest, ties to even"): Common 64-bit Decimal Floating-Point, Bankers 64-bit Decimal Floating-Point,
Common 128-bit Floating-Point, and Bankers 128-bit Decimal Floating-Point, respectively. In the following chart these types are denoted as `Common64`,
`Bankers64`, `Common128`, and `Bankers128`, respectively:

  *****************************************************************************
    C64 [s:String] => Common64
    D64 [s:String] => Bankers64
    C128[s:String] => Common128
    D128[s:String] => Bankers128

  evaluates to a Decimal Floating-Point object that represents the value specified by the argument in the conventional decimal notation (in the corresponding
  format and using the corresponding rounding mode):

    ("+" | "-" | ) (<digit> <digit>* ("." <digit>* | ) | "." <digit> <digit>*) (("e" | "E") ("+" | "-" | ) <digit> <digit>* | )

  where

    <digit> ->  "0" | "1" | "2" | ... | "9"

  *****************************************************************************
    C64 [i:Integer] => Common64
    D64 [i:Integer] => Bankers64
    C128[i:Integer] => Common128
    D128[i:Integer] => Bankers128

  --- equivalent to `C64[Str[i]]`, `D64[Str[i]]`, `C128[Str[i]]`, or `D128[Str[i]]`, respectively

### Type predicates ############################################################
    IsC64 [object] => Boolean
    IsD64 [object] => Boolean
    IsC128[object] => Boolean
    IsD128[object] => Boolean

### Polymorphic operations #############################################################################################

In the following chart `Decimal` refers to the underlying Decimal Floating-Point type, and `t` stands for the corresponding constructor (i.e., either `C64`,
`D64`, `C128`, or `D128`):

  *****************************************************************************
    x + y:Decimal => Decimal

  --- addition

  *****************************************************************************
    x - y:Decimal => Decimal

  --- subtraction

  *****************************************************************************
    x * y:Decimal => Decimal

  --- multiplication

  *****************************************************************************
    x / y:Decimal => Decimal

  --- division

  *****************************************************************************
    Neg[x] => Decimal, ~x => minus-x:Decimal

  --- negation (unary minus)

  *****************************************************************************
    x.Fma[y:Decimal; z:Decimal] => Float

  --- fused multiply-add operation --- computes `y`*`z`+`x` avoiding undue loss of precision

  *****************************************************************************
    x == y => Boolean

  --- comparison for equality

  *****************************************************************************
    x <> y => Boolean

  --- comparison for inequality

  *****************************************************************************
    x < y:Decimal => Boolean

  --- comparison for "less than"

  *****************************************************************************
    x <= y:Decimal => Boolean

  --- comparison for "less than or equal"

  *****************************************************************************
    x > y:Decimal => Boolean

  --- comparison for "greater than"

  *****************************************************************************
    x >= y:Decimal => Boolean

  --- comparison for "greater than or equal"

  *****************************************************************************
    Order[x; y:Decimal] => Integer

  --- total order/equivalence relation --- `~1` iff `x` is ordered before `y`, `1` iff `y` is ordered before `x`, and `0` otherwise

  *****************************************************************************
    Abs[x] => Decimal

  --- absolute value (magnitude, modulus)

  *****************************************************************************
    Exp[x] => Decimal

  --- base-e exponential of the argument

  *****************************************************************************
    Log[x] => Decimal

  --- base-e (natural) logarithm of the argument

  *****************************************************************************
    Log[base; x:Decimal] => Decimal

  --- equivalent to `Log[x] / Log[base]`

  *****************************************************************************
    Log10[x] => Decimal

  --- computes the base-10 logarithm of the argument with a greater accuracy than `Log[t[10]; x]` does

  *****************************************************************************
    Sqr[x] => Decimal

  --- square (`x`<sup>2</sup>, `x`*`x`)

  *****************************************************************************
    Sqrt[x] => Decimal

  --- square root of the argument

  *****************************************************************************
    x.Pow[y:Decimal] => Decimal

  --- computes `x` raised to the power `y`

  *****************************************************************************
    Trunc[x] => Decimal

  --- evaluates to the argument with the fractional part discarded

  *****************************************************************************
    Round[x] => Decimal

  --- evaluates to the argument rounded to the nearest integral value, half ties toward infinity, e.g.:

    (Round[t[".5"]] == t[1]) & (Round[~t[".5"]] == ~t[1])

  *****************************************************************************
    Floor[x] => Decimal

  --- evaluates to the nearest integral value less than or equal to the argument

  *****************************************************************************
    Ceil[x] => Decimal

  --- evaluates to the nearest integral value greater than or equal to the argument

  *****************************************************************************
    Int[x] => Integer

  --- `Trunc[x]` represented as an object of type `Integer`

  *****************************************************************************
    Str[x] => String

  --- decimal representation, e.g.:

    Str[C128["99.90"]] == "99.90"

  *****************************************************************************
    Quantize[x; y:Decimal] => Decimal

  --- quantize operation as specified in IEEE 754-2008

  ******************************************************************************
    Clone[x] => just-x:Decimal, DeepClone[x] => just-x:Decimal

  --- evaluates to an (initially) unshared Decimal Floating-Point object that represents the same value as the argument

### Exceptions #################################################################
* `Overflow` --- the rounded mathematical result of the operation does not fit into the destination floating-point format (e.g., when evaluating
  `Exp[F64[1000]]`)
* `DivisionByZero` --- the mathematical result of the operation is undefined and the function or operation has a pole for the exact value of the argument (e.g.,
  when evaluating `F64[1] / F64[0]` or `Log[F64[0]]`)
* `Undefined` --- in any other case when the mathematical result of the operation is undefined (e.g., when evaluating `F64[0] / F64[0]`, `Sqrt[~F64[1]]`, or
  `Log[~F64[1]]`)
* `SyntaxError` --- a string representation or format specifier is malformed (has invalid syntax)
* `UnrecognizedOperation`
* `InvalidInvocation`
* `TypeMismatch`
* `HeapExhausted`
* `StackOverflow`
* `LimitExceeded`


Unsigned
----------------------------------------------------------------------------------------------------------------------------------------------------------------

The type `Unsigned` corresponds to the (finite) set of integral numbers in the range 0 thru 2<sup>32</sup>-1 (i.e., 0 thru 4294967295), inclusive. However, in
comparison to the case of integers, all basic arithmetic operations on `Unsigned` values are performed by modulo 2<sup>32</sup>, and the standard library also
provides operations defined in terms of a positional binary representation (i.e, so-called bitwise operations). Like most abstract mathematical entities,
objects of type `Unsigned` are [immutable].

### Constructors #######################################################################################################

  ******************************************************************************

    U32[x] => Unsigned

  --- `I48[x].Mod[4294967296]` represented as an object of type `Unsigned`

### Type predicates ############################################################
    IsU32[object] => Boolean

### Polymorphic operations #############################################################################################

  ******************************************************************************
    x + y:Unsigned => Unsigned

  --- addition by modulo 2<sup>32</sup>

  ******************************************************************************
    x - y:Unsigned => Unsigned

  --- subtraction by modulo 2<sup>32</sup>

  ******************************************************************************
    x * y:Unsigned => Unsigned

  --- multiplication by modulo 2<sup>32</sup>

  ******************************************************************************
    x / y:Unsigned => Unsigned, x.Div[y:Unsigned] => Unsigned

  --- integer division --- the fractional part of the real result is truncated, e.g.:

      U32[8] / U32[3] == U32[2]

  ******************************************************************************
    x.Rem[y:Unsigned] => Unsigned, x.Mod[y:Unsigned] => Unsigned

  --- remainder of `(/)` --- equivalent to

    {unless U32[0] <> y signal Undefined else x - x / y * y}

  ******************************************************************************
    Neg[x] => Unsigned

  --- negation --- two's complement (equivalent to `~x + U32[1]`)

  ******************************************************************************
    x == y => Boolean

  --- comparison for equality

  ******************************************************************************
    x <> y => Boolean

  --- comparison for inequality

  ******************************************************************************
    x < y:Unsigned => Boolean

  --- comparison for "less than"

  ******************************************************************************
    x <= y:Unsigned => Boolean

  --- comparison for "less than or equal"

  ******************************************************************************
    x > y:Unsigned => Boolean

  --- comparison for "greater than"

  ******************************************************************************
    x >= y:Unsigned => Boolean

  --- comparison for "greater than or equal"

  ******************************************************************************
    Order[x; y:Unsigned] => Integer

  --- total order/equivalence relation --- equivalent to

    {if x < y then ~1 else: if x > y then 1 else 0}

  ******************************************************************************
    Abs[x] => just-x:Unsigned

  --- evaluates to the argument itself

  ******************************************************************************
    x & y:Unsigned => Unsigned

  --- bitwise "and"

  ******************************************************************************
    x | y:Unsigned => Unsigned

  --- bitwise "or"

  ******************************************************************************
    ~x => Unsigned

  --- bitwise "not" (bitwise negation, complement)

  ******************************************************************************
    x.Xor[y:Unsigned] => Unsigned

  --- bitwise exclusive "or"

  ******************************************************************************
    Lsh[x; n:Integer] => Unsigned

  --- logical bit shift (computes `x`*2<sup>n</sup> mod 2<sup>32</sup>)

  ******************************************************************************
    Ash[x; n:Integer] => Unsigned

  --- arithmetic bit shift, left for positive `n`, right for negative `n`, the MSB[^6] counts as the "sign" bit

[^6]: most significant bit

  ******************************************************************************
    Rot[x; n:Integer] => Unsigned

  --- bit rotation, left for positive `n`, right for negative `n`, and

    Rot[x; n] == Rot[x; n.Rem[32]]

  ******************************************************************************
    Ctz[x] => Unsigned

  --- trailing zeros count, for non-zero arguments

  ******************************************************************************
    Clz[x] => Unsigned

  --- leading zeros count, for non-zero arguments

  ******************************************************************************
    Log2[x] => Unsigned

  --- equivalent to `U32[31] - Clz[x]`

  ******************************************************************************
    C1s[x] => popcount:Unsigned

  --- ones count (population count)

  ******************************************************************************
    Str[x] => String

  --- hexadecimal representation, e.g.:

    Str[U32[123]] == "0x0000006F"

  *******************************************************************************
    Str[x; format:String] => String

  --- argument string representation formatted according to a [C]/[POSIX] `printf` [format specifier] for the `unsigned` type, with the leading `%` character
  stripped, e.g.:

    Str[U32[10]; "3u"] == " 10", Str[U32[10]; "04X"] == "000A"

  ******************************************************************************
    Int[x] => Integer

  --- value represented as an object of type Integer

  ******************************************************************************
    Clone[x] => just-x:Unsigned, DeepClone[x] => just-x:Unsigned

  --- evaluates to the argument itself

### Exceptions #################################################################
* `DivisionByZero`
* `Undefined`
* `SyntaxError`
* `UnrecognizedOperation`
* `InvalidInvocation`
* `TypeMismatch`
* `HeapExhausted`
* `StackOverflow`
* `LimitExceeded`


Complex
----------------------------------------------------------------------------------------------------------------------------------------------------------------

An object of type `Complex` (or more accurately, Binary Floating-Point Complex) consists of a pair of two Binary Floating-Point values in the same
floating-point format, which represent a complex number in Cartesian form. Like most abstract mathematical entities, objects of type Binary Floating-Point
Complex are immutable.

### Constructors #######################################################################################################

Since there are two floating-point data types, there are also two corresponding complex data types. In the following chart these types are denoted as
`Complex32` and `Complex64`:

  ******************************************************************************
    Z64[re:Float64; im:Float64] => Complex64
    Z32[re:Float32; im:Float32] => Complex32

  --- evaluates to a 64-bit or 32-bit Floating-Point Complex object, respectively, with the specified real and imaginary part

  ******************************************************************************
    Z64[re:Float64] => Complex64
    Z32[re:Float32] => Complex32

  --- evaluates to a 64-bit or 32-bit Floating-Point Complex object, respectively, that represents the same floating-point value as the argument (with the
  imaginary part set to positive zero)

  ******************************************************************************
    Z64[s:String] => Complex64
    Z32[s:String] => Complex32

  --- evaluates to a 64-bit or 32-bitFloating-Point Complex object, respectively, that represents a value specified by the argument in the conventional decimal
  notation, which can include both real and imaginary parts, separated by a comma:

    ("+" | "-" | ) (<digit> <digit>* ("." <digit>* | ) | "." <digit> <digit>*)
    (("e" | "E") ("+" | "-" | ) <digit> <digit>* | ) (","
    ("+" | "-" | ) (<digit> <digit>* ("." <digit>* | ) | "." <digit> <digit>*)
    (("e" | "E") ("+" | "-" | ) <digit> <digit>* | ) | )

  where

    <digit> ->  "0" | "1" | "2" | ... | "9"

  ******************************************************************************
    Z64[i:Integer] => Complex64
    Z32[i:Integer] => Complex32

  --- equivalent to `Z64[Str[i]]` or `Z32[Str[i]]`, respectively

### Type predicates ############################################################
    IsZ64[object] => Boolean
    IsZ32[object] => Boolean

### Polymorphic operations #############################################################################################

In the following chart `Complex` refers to the underlying `Complex` type, `Float` refers to the Binary Floating-Point type using the corresponding
floating-point format, and `t` stands for the corresponding constructor (i.e., either `Z32` or `Z64`):

  ******************************************************************************
    x + y:Complex => Complex

  --- addition

  ******************************************************************************
    x - y:Complex => Complex

  --- subtraction

  ******************************************************************************
    x * y:Complex => Complex

  --- multiplication

  ******************************************************************************
    x / y:Complex => Complex

  --- division

  ******************************************************************************
    Neg[x] => Complex, ~x => minus-x:Complex

  --- negation (unary minus)

  ******************************************************************************
    Conj[x] => Complex

  --- complex conjugate

  ******************************************************************************
    x == y => Boolean

  --- comparison for equality

  ******************************************************************************
    x <> y => Boolean

  --- comparison for inequality

  ******************************************************************************
    Order[x; y:Complex] => Integer

  --- total order/equivalence relation --- objects of a `Complex` type are ordered lexicographically with respect to their Cartesian components

  ******************************************************************************
    Re[x] => Float

  --- real part

  ******************************************************************************
    Im[x] => Float

  --- imaginary part

  ******************************************************************************
    Abs[x] => Float

  --- absolute value (modulus)

  ******************************************************************************
    Arg[x] => Float

  --- argument (phase) of a complex value

  ******************************************************************************
    Exp[x] => Complex

  --- base-e exponential of the argument

  ******************************************************************************
    Log[x] => Complex

  --- base-e (natural) logarithm of the argument

  ******************************************************************************
    Log[base; x:Complex] => Complex

  --- equivalent to `Log[x] / Log[base]`

  ******************************************************************************
    Log10[x] => Complex

  --- computes the base-10 logarithm of the argument (with a greater accuracy than `Log[t[10]; x]` does)

  ******************************************************************************
    Sqrt[x] => Complex

  --- square root of the argument

  ******************************************************************************
    x.Pow[y:Complex] => Complex

  --- computes `x` raised to the power `y`

  ******************************************************************************
    Sin[x] => Complex

  --- sine

  ******************************************************************************
    Cos[x] => Complex

  --- cosine

  ******************************************************************************
    Tan[x] => Complex

  --- tangent

  ******************************************************************************
    Asin[x] => Complex

  --- arcsine

  ******************************************************************************
    Acos[x] => Complex

  --- arccosine

  ******************************************************************************
    Atan[x] => Complex

  --- arctangent

  ******************************************************************************
    Sinh[x] => Complex

  --- hyperbolic sine

  ******************************************************************************
    Cosh[x] => Complex

  --- hyperbolic cosine

  ******************************************************************************
    Tanh[x] => Complex

  --- hyperbolic tangent

  ******************************************************************************
    Asinh[x] => Complex

  --- inverse hyperbolic sine

  ******************************************************************************
    Acosh[x] => Complex

  --- inverse hyperbolic cosine

  ******************************************************************************
    Atanh[x] => Complex

  --- inverse hyperbolic tangent

  ******************************************************************************
    Str[x] => String

  --- decimal representation, e.g.:

    Str[Exp[Z64["0,-1"]]] == "5.4030230586813977e-01,-8.4147098480789650e-01"

  ******************************************************************************
    Clone[x] => just-x:Complex, DeepClone[x] => just-x:Complex

  --- evaluates to an (initially) unshared Floating-Point Complex object that represents the same value as the argument

### Exceptions #################################################################
* `Overflow` --- the rounded mathematical result of the operation does not fit into the destination floating-point format (e.g., when evaluating
  `Exp[F64[1000]]`)
* `DivisionByZero` --- the mathematical result of the operation is undefined and the function or operation has a pole for the exact value of the argument (e.g.,
  when evaluating `F64[1] / F64[0]` or `Log[F64[0]]`)
* `Undefined` --- in any other case when the mathematical result of the operation is undefined (e.g., when evaluating `F64[0] / F64[0]`, `Sqrt[~F64[1]]`, or
  `Log[~F64[1]]`)
* `SyntaxError` --- a string representation or format specifier is malformed (has invalid syntax)
* `UnrecognizedOperation`
* `InvalidInvocation`
* `TypeMismatch`
* `HeapExhausted`
* `StackOverflow`
* `LimitExceeded`


[immutable]:              CoreSemantics.html#h:immutabilitymutability-of-objects "Immutability/mutability of objects"
[literal representation]: Syntax.html#h:literals                                 "Literals"

[C]:                //en.wikipedia.org/wiki/C_(programming_language "Wikipedia: C"
[POSIX]:            //en.wikipedia.org/wiki/POSIX                   "Wikipedia: POSIX"
[format specifier]: //en.wikipedia.org/wiki/printf                  "Wikipedia: printf"

{%endraw%}{%include spec_footer.md%}
