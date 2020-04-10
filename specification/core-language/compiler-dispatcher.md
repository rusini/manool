---
# specification/core-language/compiler-dispatcher.md
title:   Compiler Dispatcher &mdash; Specification
updated: 2019-12-21
---

<aside markdown="1" class="right">

See also:
  * [General](General#start)
  * [Core Semantics](CoreSemantics#start)

</aside>

{%include spec_header.md%}{%raw%}


Metanotation
----------------------------------------------------------------------------------------------------------------------------------------------------------------

Two metalanguages are used in this section:
  * plain English and
  * the language of MANOOL [expressions] (whereby involving a metacircular specification).

All MANOOL expressions in the metacircular specification are to be considered in the binding environment of the MANOOL [standard library].

[expressions]:      /specification/core-language/semantic-concepts#h:forms-expressions-control-flow  "Forms, expressions, control flow"
[standard library]: /specification/standard-library/#start                                           "Standard Library"


In Plain English
----------------------------------------------------------------------------------------------------------------------------------------------------------------

Run-time code, which is the output of the semantic analysis and code generation [translation phase], is represented by a _compiled entity_, which internally (in
its turn) may contain other compiled entities. Incidentally, compiled entities are also what identifiers are [bound] to in a binding environment.

A kind of object-oriented approach is used in this specification --- compiled entities are considered to be active agents (represented by a MANOOL value/object
for the purposes of the metacircular description) capable of providing services whenever the abstract machine asks them to.

The abstract machine has a (very) simple and compact semantic analysis and code generation _core dispatcher_. To compile an expression, the dispatcher first
determines what kind of AST represents it:

1. If it is a symbol (explicitly) bound to a compiled entity in the active binding environment, that compiled entity becomes the result of compilation.

2. Otherwise, if it is an [integer], a [string], or a [symbol], and the symbol starts with a character other than an ASCII lowercase letter (`a` thru `z`), the
   abstract machine constructs a compiled entity that represents a literal value.

3. Otherwise, if it is a [compound expression], the dispatcher compiles (recursively) the head subexpression and then asks the resulting compiled entity to
   compile the whole expression.

4. Otherwise, if it is a special MANOOL value that encapsulates a compiled entity, which is produced by a [`#`-expression] (and for which the predicate `IsCode`
   returns `True`), then the encapsulated compiled entity becomes the result of compilation.

5. As a fallback, the dispatcher reports an error as appropriate.

Note that checking the presence and placement of keywords (such as `then`, `else`, `do`, etc.) is performed, if needed, by compiled entities rather than
directly by the dispatcher. Also, a compiled entity, when asked to compile an expression, may in turn call the dispatcher for subexpressions[^b1] and possibly
specify a different active binding environment to compile in.

[^b1]: .. and occasionally, even for expressions constructed artificially on-the-fly ...

[translation phase]:   /specification/general#h:translation-overview                                                        "Translation Overview"
[bound]:               /specification/core-language/semantic-concepts#h:bindings-and-scopes-binding-environments "Bindings and scopes, binding environments"
[integer]:             /specification/standard-library/basic-data-types#h:integer                                           "Integer Type"
[string]:              /specification/standard-library/basic-data-types#h:string                                            "String Type"
[symbol]:              /specification/standard-library/basic-data-types#h:symbol                                            "Symbol Type"
[compound expression]: /specification/core-language/semantic-concepts#h:forms-expressions-control-flow                      "Forms, expressions, control flow"
[`#`-expression]:      /specification/standard-library/custom-notation-syntactic-macros-metaprogramming#h:sharp-expressions "#-expressions"

Metacircular Specification
----------------------------------------------------------------------------------------------------------------------------------------------------------------

For the purposes of illustration, let's assume that a compiled entity recognizes the following polymorphic operations (beyond the standard ones):

* `IsRvalue` --- tell whether the compiled entity corresponds to an r-value expression

* `IsLvalue` --- tell whether the compiled entity corresponds to an l-value expression (which shall imply a positive answer to the above question as well)

* `Compile` --- given an active binding environment represented by a mapping `SymTab`, compile the specified compound expression (whose head subexpression
  corresponds to the compiled entity) to produce as a result another compiled entity

* `Execute` --- given an evaluation context `Ctx`, evaluate the expression the compiled entity represents and produce a resulting value (applicable only in case
  of r-value expressions)

* `ExecIn` --- given an evaluation context `Ctx` and a value `Val`, store the value into the location the compiled entity represents (applicable only in case of
  l-value expressions)

* `ExecOut` --- given an evaluation context `Ctx`, move out the current value from the location the compiled entity represents (applicable only in case of
  l-value expressions) to produce as a result the value moved out

The core dispatcher algorithm described above may be specified more formally using the following metacircular description (in MANOOL):[^c1]

    { let rec
      { Compile =
        { proc { Form; SymTab } as
        : if IsSym[Form] & SymTab.Exists[Form] then SymTab[Form] else -- bound identifier
        : if IsInt[Form] | IsStr[Form] | IsSym[Form] &
          { do True after
          : if (Str[Form] <> "") & (Str[Form][0] >= "a"[0]$) & (Str[Form][0] <= "z"[0]$) then
          : signal CompileError with "unbound keyword (nested in this context)"
          }
          then -- literal value
          { let { _Form } in
          : object { _Form = Form } with
            IsRvalue = {proc {_} as True}
            IsLvalue = {proc {_} as False}
            Compile' = CompileApply
            Execute  = {proc {Self; _} as Self[_Form]@}
          }
          else
        : if IsList[Form] & (Size[Form] <> 0) then Compile[Form[0]; SymTab].(Compile')[Form; SymTab] else -- compound expression
        : if IsCode[Form] then Form[_Entity]@ else -- a result of e# (used for metaprogramming)
        : signal CompileError with "invalid form"
        }
      }
      in
    : export Compile
    }

[^c1]: Understanding a metacircular specification for a language _L_ requires prior knowledge of the language _L_, which raises a question about its utility.
       However, normally only *partial* knowledge is required, and this roughly corresponds to how children learn communicative systems (i.e., natural
       languages) from zero.

As a matter of illustration, a compiled entity bound to the `if` keyword could be constructed by evaluating the expression

    { object {} with
      -- Classification
      IsRvalue = {proc {_} as False}
      IsLvalue = {proc {_} as False}
      -- Compilation
      Compile' =
      { proc { _; Form; SymTab } as
      : if (Size[Form] >= 6) & (Form[2] == then') & (Form[4] = else') then
        { let { _Cond; _Body1; _Body2 } in
        : object
          { _Cond  = CompileRvalue[Form[1]; SymTab]
            _Body1 = CompileRvalue[Form[3]; SymTab]
            _Body2 = CompileRvalues[Form.Elems[Range[5; Size[Form]]]; SymTab]
          }
          with
          -- Classification
          IsRvalue = {proc {_} as True}
          IsLvalue = {proc {Self} as Self[_Body1]@.IsLvalue[] & Self[_Body2]@.IsLvalue[]}
          -- Execution
          Execute = {proc {Self; Ctx} as Execute[Self[{if Execute[Self[_Cond]@; Ctx] then _Body1 else _Body2}]@; Ctx]}
          ExecIn  = {proc {Self; Val; Ctx} as ExecIn[Self[{if Execute[Self[_Cond]@; Ctx] then _Body1 else _Body2}]@; Val; Ctx]}
          ExecOut = {proc {Self; Ctx} as ExecOut[Self[{if Execute[Self[_Cond]@; Ctx] then _Body1 else _Body2}]@; Ctx]}
          -- Compilation
          Compile' = CompileApply
        }
        else
      : if (Size[Form] >= 4) & (Form[2] == then') then
        { let { _Cond; _Body } in
        : object
          { _Cond = CompileRvalue[Form[1]; SymTab]
            _Body = CompileRvalues[Form.Elems[Range[3; Size[Form]]]; SymTab]
          }
          with
          -- Classification
          IsRvalue = {proc {_} as True}
          IsLvalue = {proc {_} as False}
          -- Execution
          Execute = {proc {Self; Ctx} as: if Execute[Self[_Cond]@; Ctx] then Execute[Self[_Body]@; Ctx]}
          -- Compilation
          Compile' = CompileApply
        }
        else
      : signal CompileError with "invalid form"
      }
    }


{%endraw%}{%include spec_footer.md%}
