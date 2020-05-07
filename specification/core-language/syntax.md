---
# specification/core-language/syntax.md
title:   Syntax &mdash; Specification
updated: 2019-12-22
---

<aside markdown="1" class="right">

See also:
  * [General](/specification/general#start)
  * [Semantic Concepts](/specification/core-language/semantic-concepts#start)
  * [Basic Data Types](/specification/standard-library/basic-data-types#start)
  * [Composite Data Types](/specification/standard-library/composite-data-types#start)

</aside>

{%include spec_header.md%}{%raw%}


Metanotation
----------------------------------------------------------------------------------------------------------------------------------------------------------------
[expression]:       /specification/core-language/semantic-concepts#h:forms-expressions-control-flow  "Forms, expressions, control flow"
[value]:            /specification/core-language/semantic-concepts#h:values-objects-special-entities "Values, objects, special entities"
[standard library]: /specification/standard-library/#start                                           "Standard Library"

A combination of the following metalanguages and metalinguistic formalisms is used in this section:
  * plain English,
  * MANOOL [expressions][expression] (whereby involving a kind of metacircular description),
  * _regular expressions_, and
  * a formal _context-free grammar_[^a1] with attributes (i.e., _semantic values_).

[^a1]: ... which is unambiguous and actually belongs to the LALR class of grammars ...

All MANOOL expressions involved as metacircular descriptions are to be considered in the binding environment of the MANOOL [standard library].

### Regular expressions ################################################################################################

A regular expression `regexp` that describes a _lexical category_ `<lexcat>L` (which corresponds to a _terminal symbol_ from the context-free grammar) is
provided according to the following format:

    <lexcat>L -> regexp [[ ... ]]

as in the following example:

    <lit>L -> (<letter> | "_") (<letter> | "_" | <digit>)*  [[ {if E <> "_" then MakeSym[E] else MakeSym[]} ]]

Alternatively, a lexical category may be denoted simply as `"chars"L` as in the example:

    ";"L -> ";"  [[ Nil ]]

Auxiliary (helper) definitions may also be provided, e.g.:

    <digit> -> "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9"

Inside `regexp`
  * a single literal character or a sequence thereof is enclosed in either `"` or `'` (double or single quotes),
  * a mere concatenation of subexpressions means sequencing,
  * `*` (an asterisk metaoperator) means repetition zero or more times of the preceding item,
  * `|` (a vertical bar) separates syntactic alternatives (and is read "or"),
  * `*` bind stronger than `|` and plain concatenations whereas the later bind stronger than `|`,
  * `(` and `)` (parentheses) are used for explicit grouping subexpressions, and
  * `...` (ellipses) are used to represent "obvious" omissions and have a rather informal character.

At the end of each "lexical production" a _semantic evaluation function_ as a MANOOL [expression] is provided enclosed in `[[` and `]]` whose purpose is to
describe construction of the corresponding lexical (semantic) [value] (i.e., an attribute value from the standpoint of attribute grammars) out of the lexeme
text, denoted as `E` (from "[lexical **e**lement]").

### The context-free grammar ###########################################################################################

Each production of the context-free grammar for a _nonterminal symbol_ (i.e., a _syntactic category_) `<syncat>S` is provided according to the following
format:

    <syncat>S -> rhs [[ ... ]]

where no metaoperators occur except `|` (which is just an optional shorthand notation to represent several alternative productions for the same left-hand
side).

Inside all regular expressions and productions, `""` means "&epsilon;" (i.e., the empty string) and is useful to reinforce clarity.

Again, at the end of each production a semantic evaluation function as a MANOOL [expression] may be provided enclosed in `[[` and `]]` that constructs the
corresponding semantic [value] out of the semantic values of constituents, accessible as elements of the array `A` (from "**a**ttributes"). In this case each
corresponding symbol in the right-hand-side of the production is followed by the corresponding index between `[` and `]` (brackets), as in the example:

    <args'>S  ->  <datum>S[0] ";"L <args'>S[2]  [[ MakeList[{array of A[0]} + A[2]] ]]

The default semantic evaluation function is assumed to be simply `A[0]`.


Lexical Structure
----------------------------------------------------------------------------------------------------------------------------------------------------------------
[lexical **e**lement]: #h:lexical-structure "Below: Lexical Structure"

During the [translation phase] of lexical analysis, the abstract machine decomposes the input character string into a concatenation of _lexical elements_ and
_lexical separators_ (or just _separators_, for short). The later are ignored except as they serve to separate the former. At least one separator is required
between an integer or symbol [literal][literals] and a nearby integer or symbol literal.[^b1]

[^b1]: This rule ensures that the decomposition is unique.

A lexical separator is either a [whitespace] character or a [comment][comments].

Lexical elements are classified into
  * [literals] ([integer][integer literals], [string][string literals], and [symbol][symbol literals]),
  * [infix operators] (equivalence/association, relational, additive, and multiplicative),
  * [unary operators] (prefix and postfix), and
  * [delimiters and punctuators][delimiters, punctuators].

### Source character set and encoding ##################################################################################
[whitespace]: #h:source-character-set-and-encoding "Below: Source character set and encoding"

This specification deliberately makes no provisions for particular character sets or encodings to be used to interpret program units as text files. Moreover, it
is generally possible to represent MANOOL programs as series of bytes (octets) in a manner agnostic to character set and character encoding. Such
representation, however, shall cover the ASCII character set and shall be backward compatible with ASCII in respect to character encoding. As an extreme
example, it would be perfectly valid to assume the UTF-8 character encoding for some source code fragments and ISO-8859-1 for other fragments, even in the same
source file.

Program units are composed only of the following graphical ASCII characters:

* 26 uppercase letters:

      A B C D E F G H I J K L M N O P Q R S T U V W X Y Z

* 26 lowercase letters:

      a b c d e f g h i j k l m n o p q r s t u v w x y z

* 10 decimal digits:

      0 1 2 3 4 5 6 7 8 9

* 30 special characters:

      ! " # $ % & ' ( ) * + - . / : ; < = > ? @ [ \ ] ^ _ { | } ~

plus the following _whitespace_ ASCII characters:
  * `SP` --- space;
  * `HT` --- tab;
  * `VT` --- vertical tab;
  * `FF` --- form feed;
  * `CR` --- carriage return;
  * `LF` --- line feed (_line separator_);

except that [string literals] and [comments] may contain any additional characters (regardless of encoding).

As a special case, the abstract machine considers a zero byte (corresponding to the ASCII `NUL` character) to be an optional end-of-file marker (effectively
ignoring it and the rest of the file). A line separator character at end of file in a program unit is recommended but not required as per this specification.

Note that MANOOL is a case-sensitive language, so the abstract machine considers lowercase and uppercase letters as distinct for all purposes. Also note that
the following two characters are illegal in program units anywhere outside of string literals and comments and are reserved for future use:
  * <code>,</code> --- comma;
  * <code>`</code> --- ASCII grave accent or backquote.

### Literals ###########################################################################################################
[literals]: #h:literals "Below: Literals"

In this section the following helper definitions hold:

    <letter> ->  "A" | "B" | "C" | ... | "Z" | "a" | "b" | "c" | ... | "z"
    <digit>  ->  "0" | "1" | "2" | "3" | "4" | "5" | "6" | "7" | "8" | "9"
    <any char except '"' and LF> -> ...

#### Integer literals
[integer literals]: #h:integer-literals                                        "Below: Integer literals"
[integer]:          /specification/standard-library/basic-data-types#h:integer "Integer Type"

An _integer literal_ consists of one or more decimal digits and denotes a lexical value of type [Integer] by using the conventional decimal notation:

    <lit>L -> <digit> <digit>*  [[ Int[E] ]]

Examples:

    0   1   2   5   10   123   2018   140737488355327

#### String literals
[string literals]: #h:string-literals                                        "Below: String literals"
[string]:          /specification/standard-library/basic-data-types#h:string "String Type"

A _string literal_ consists of zero or more characters --- other than double quotes (`"`) and line separators --- enclosed in double quotes. With the quotes
stripped off, a string literal denotes a lexical value of type [String], as is:

    <lit>L -> '"' <any char except '"' and LF>* '"'  [[ E[Range[1; Size[E] - 1]] ]]

Examples:

    ""   "foo"   "This is a string"   "manool.org.18/std/0.5/all"

Alternatively, `\}` and `\{` may be used instead of double quotes.[^b2] In this case the lexical value may contain anything except `\{`. Thus,

    \}
      <p>This is a paragraph.</p>
    \{

is equivalent to

    (Lf + Sp + Sp + "<p>This is a paragraph.</p>" + Lf)$

[^b2]: That's right --- a closing brace is used to start a literal and an opening brace to end it. This notation is convenient for "embedding" MANOOL in text
       documents as a "macro" language in the spirit of PHP, JSP, ASP, etc.

#### Symbol literals
[symbol literals]: #h:symbol-literals                                        "Below: Symbol literals"
[symbol]:          /specification/standard-library/basic-data-types#h:symbol "Symbol Type"

A _symbol literal_ consists of one or more letters, digits, and underscore characters (`_`) and starts with a letter or underscore. Each occurrence in a program
unit of a symbol literal denotes a lexical value of type [Symbol]. For any symbol literal distinct from a single underscore, the value is obtained by
constructing a symbol out of the literal (i.e., straight out of the text); otherwise, the value is represented by a unique uninterned symbol generated by the
abstract machine during the lexical analysis phase on each occurrence of the literal:

    <lit>L -> (<letter> | "_") (<letter> | "_" | <digit>)*  [[ {if E <> "_" then MakeSym[E] else MakeSym[]} ]]

Examples:

    A   WriteLine   extern   Log10   _   _Num

Note that negative integers, some categories of strings, extremely long strings, and some categories of symbols have no literal representation whatsoever.

### Infix operators ####################################################################################################
[infix operators]: #h:infix-operators "Below: Infix operators"

An _infix operator_ is similar to a symbol literal, although unlike the later it consists of one or two special characters and is classified from the syntactic
analysis standpoint as either `<equ op>L`, `<rel op>L`, `<add op>L`, or `<mul op>L`:

* Equivalence/association operator:

      <equ op>L -> "="  [[ Sym[E] ]]

* Relational operators:

      <rel op>L -> "==" | "<>" | "<" | "<=" | ">" | ">="  [[ Sym[E] ]]

* Additive operators:

      <add op>L -> "+" | "-" | "|"  [[ Sym[E] ]]

* Multiplicative operators:

      <mul op>L -> "*" | "/" | "&"  [[ Sym[E] ]]

### Unary operators ####################################################################################################
[unary operators]: #h:unary-operators "Below: Unary operators"

A _unary operator_ is similar to a symbol literal, although unlike the later it consists of a single special character and is classified from the syntactic
analysis standpoint as either `<pref op>L` or `<post op>L`:

* Prefix operator:

      <pref op>L -> "~"  [[ Sym[E] ]]

* Postfix operators:

      <post op>L -> "!" | "#" | "$" | "%" | "'" | "?" | "@" | "^"  [[ Sym[E] ]]

### Delimiters, punctuators ############################################################################################
[delimiters, punctuators]: #h:delimiters-punctuators "Below: Delimiters, punctuators"

The only _delimiter_ in MANOOL is `;`:

    ";"L -> ";"  [[ Nil ]]

_Punctuators_ in MANOOL are

    (  )  .  :  [  ]  {  }

Delimiters and punctuators lack any meaningful lexical value and serve only for grouping syntactically other values and overriding operator precedence and
associativity.

### Comments ###########################################################################################################
[comments]: #h:comments "Below: Comments"

MANOOL syntax allows for two kinds of comments: _line comments_ and _block comments_.

#### Line comments

A line comment starts with two adjacent `-` (ASCII hyphen-minus or dash) characters and extends up to the nearest end of line. Here is an example of a line
comment:

    -- This is a line comment

#### Block comments

Block comments may be recursively nested inside one another. A block comment starts with a combination of `/*` (ASCII solidus or slash plus asterisk) characters
and extends up through the matching combination `*/`, which shall not immediately precede an asterisk. Inside a block comment, combinations `/*` and `*/` do not
start or end a nested comment when encountered within what would look like a line comment or string literal if it had occurred outside of a comment. This
applies even to malformed string literals implicitly terminating at end of line.[^b3] This is a complete, properly terminated block comment (provided no
asterisk immediately follows it):

    /* This is a block comment
      */*** This is a nested comment
        -- This is a line ***/comment/***
        Out.WriteLine["Comments terminate on */ and start on /*"]
        "Malformed ***/string literal/*** ends here ->
      end of comment ***/
    end of comment */

[^b3]: Thus, block comments are ideal for temporary commenting out fragments of source code (e.g., while debugging) but may be used for other purposes as well.


Syntactic Analysis
----------------------------------------------------------------------------------------------------------------------------------------------------------------
[array]: /specification/standard-library/composite-data-types#h:array "Array Type"

On the second [translation phase], the abstract machine parses the input in the form of string of terminal symbols according to the context-free grammar
provided below, builds up a parse tree, and then constructs the AST intermediate representation.

Context-free grammar (with attributes and semantic evaluation functions):

    <start>S  ->  <datum>S
    <datum>S  ->  <datum'>S
    <datum>S  ->  <datum'>S[0] <equ op>L[1] <datum'>S[2]  [[ MakeList[{array of A[1]; A[0]; A[2]}] ]]
^
    <datum'>S ->  <simple>S
    <datum'>S ->  <simple>S[0] <rel op>L[1] <simple>S[2]  [[ MakeList[{array of A[1]; A[0]; A[2]}] ]]
^
    <simple>S ->  <term>S
    <simple>S ->  <simple>S[0] <add op>L[1] <term>S[2]  [[ MakeList[{array of A[1]; A[0]; A[2]}] ]]
^
    <term>S   ->  <factor>S
    <term>S   ->  <term>S[0] <mul op>L[1] <factor>S[2]  [[ MakeList[{array of A[1]; A[0]; A[2]}] ]]
^
    <factor>S ->  <prim>S
    <factor>S ->  <pref op>L[0] <factor>S[1]  [[ MakeList[{array of A[0]; A[1]}] ]]
^
    <prim>S   ->  <prim'>S
    <prim>S   ->  <prim>S[0] "["L <args>S[2] "]"L  [[ MakeList[{array of A[0]} + A[2]] ]]
    <prim>S   ->  <prim>S[0] "."L <prim'>S[2] "["L <args>S[4] "]"L  [[ MakeList[{array of A[2]; A[0]} + A[4]] ]]
    <prim>S   ->  <prim>S[0] <post op>L[1]  [[ MakeList[{array of A[1]; A[0]}] ]]
^
    <prim'>S  ->  <lit>L[0]  [[ A[0] ]]  |  "("L <op>S[1] ")"L  [[ A[1] ]]
    <prim'>S  ->  "{"L <list>S[1] "}"L  [[ A[1] ]]  |  "("L <datum>S[1] ")"L  [[ A[1] ]]
^
    <op>S     ->  <equ op>L | <rel op>L | <add op>L | <mul op>L | <pref op>L | <post op>L
^
    <args>S   ->  <args'>S  |  ""  [[ MakeList[{array}] ]]
^
    <args'>S  ->  <datum>S[0] <args>S[1]  [[ MakeList[{array of A[0]} + A[1]] ]]
    <args'>S  ->  <datum>S[0] ";"L <args'>S[2]  [[ MakeList[{array of A[0]} + A[2]] ]]
^
    <list>S   ->  <list'>S  |  ""  [[ MakeList[{array}] ]]
^
    <list'>S  ->  <datum>S[0] <list>S[1]  [[ MakeList[{array of A[0]} + A[1]] ]]
    <list'>S  ->  <datum>S[0] ";"L <list'>S[2]  [[ MakeList[{array of A[0]} + A[2]] ]]
    <list'>S  ->  <datum>S[0] ":"L <list'>S[2]  [[ MakeList[{array of A[0]; A[2]}] ]]

  where `MakeList` is some (pure) function, and for any [array] `A` of ASTs, the following condition is met:

    MakeList[A].IsList[] & (Size[MakeList[A]] == Size[A]) & {for {E1 = MakeList[A]; E2 = A} all E1 == E2}


[translation phase]: /specification/general#h:translation-overview "Translation Overview"

{%endraw%}{%include spec_footer.md%}
