---
title:   Lesson 2 -- Tutorial
updated: 2020-05-31
---

<aside markdown="1" class="right">
Lesson 2 of 5 -- Tutorial
=========================
+ **[Previous Lesson](lesson-1#start)**
+ **[Next Lesson](lesson-3#start)**
+ [Lesson 4](lesson-4#start)
+ [Lesson 5](lesson-5#start)
</aside>

{%include page_header.md%}{%raw%}


Factorial
----------------------------------------------------------------------------------------------------------------------------------------------------------------

The usual "Hello World" analog for functional languages consists of a recursive function definition for calculating factorial of a number. The following
variation for MANOOL incorporates in addition some test code:

    -- Factorial -- recursive version in MANOOL-ish ("cascading") notation
    { {extern "manool.org.18/std/0.5/all"} in
    : let rec
      { Fact = -- compile-time constant binding
        { proc { N } as -- precondition: 0 <= N
        : if N == 0 then 1 else
          N * Fact[N - 1]
        }
      }
      in
      Out.WriteLine["Factorial of 10 = " Fact[10]]
    }

And the following equivalent program (up to AST) is intended to make the syntactic structure of the above program a bit more apparent:

    -- Factorial -- recursive version in conventional notation (equivalent to the above code, up to AST)
    { {extern "manool.org.18/std/0.5/all"} in
      { let rec
        { Fact = -- compile-time constant binding
          { proc { N } as -- precondition: 0 <= N
            { if N == 0 then 1 else
              N * Fact[N - 1]
            }
          }
        }
        in
        Out.WriteLine["Factorial of 10 = " Fact[10]]
      }
    }

  (here another piece of syntactic sugar is demonstrated -- any two constructs
`{`_a_<sub>0</sub> _a_<sub>1</sub> ... _a_<sub>_m_-1</sub> `{`_b_<sub>0</sub> _b_<sub>1</sub> ... _b_<sub>_n_-1</sub>`}``}` and
`{`_a_<sub>0</sub> _a_<sub>1</sub> ... _a_<sub>_m_-1</sub>`:` _b_<sub>0</sub> _b_<sub>1</sub> ... _b_<sub>_n_-1</sub>`}` are always equivalent one to another).

Output:

    Factorial of 10 = 3628800

#### How it works

1. The expression `{proc {N} as ...}` resembles a lambda-expression in many languages, where `N` would specify a parameter and the expression that follows `as`
   would be the body. The whole expression evaluates to a procedure, which returns the result of evaluation of the body.[^a1]

2. During compilation of the expression `{let rec {Fact = ...} in ...}`, a binding between `Fact` and the entity specified on the right-hand side of the infix
   operator `=` is injected into the scope that follows `in`.[^a2] Since we use `let rec` and not just `let` here, the right-hand side expression is also
   included in the scope of `Fact`, so we can refer to it recursively.

3. The construct `{if ... then ... else ...}` is a conditional expression here. During its evaluation either of the two branches is evaluated depending on
   whether the condition specified between `if` and `then` holds and producing the result for the whole expression.

[^a1]: In MANOOL, a `proc`-expression does not implicitly capture temporary variables from its initial binding environment (by default, the body of
       `proc`-expression is simply excluded from the scope of such variables). This is due to a deliberate language design decision.

[^a2]: That entity is obtained in full at compile-time.

### Iterative version ##################################################################################################

Although MANOOL has a functional core, it is a multiparadigm language, for which an iterative version of the factorial function, which uses a `while`-loop (or
`for`-loop), may be more appropriate:[^a3]

    -- Factorial -- iterative version (in MANOOL, this is probably more appropriate for factorial)
    { {extern "manool.org.18/std/0.5/all"} in
    : let
      { Fact = -- compile-time constant binding
        { proc { N } as -- precondition: 0 <= N
        : var { Res = 1 } in -- variable binding
        : do Res after -- return result
        : while N <> 0 do -- loop while N not equals zero
          Res = N * Res; N = N - 1
        }
      }
      in
      Out.WriteLine["Factorial of 10 is "; Fact[10]]
    }

  (the output is the same as above).

[^a3]: The MANOOL specification does not require tail-call optimizations, though this is not the only reason.

#### How it works

1. During compilation of the expression `{var {Res = 1} in ...}`, the body expression(s), which follow `in`, are considered in a binding environment with a
   temporary variable named `Res` injected.[^a4] The variable is initialized to `1` just before evaluating the body expressions.

2. The expression `{do Res after ...}` is equivalent to `{do ...; Res}`, which is evaluated by evaluating its constituents one by one, and thus this expression
   evaluates to `Res`.

3. The expression `{while ... do ...}` is a traditional `while`-loop. During its evaluation, the body expression(s), which follow `do`, are evaluated
   repetitively, one by one, while the pre-condition specified between `while` and `do` holds.

4. `Res = N * Res` and `N = N - 1` are assignment expressions. As a side effect of an evaluation of such expression, the current value of the location specified
   on the left-hand side of the `=` operator is replaced with the value specified on the right-hand side of the `=` operator.

[^a4]: Temporary variables in MANOOL (including procedure parameters) are statically scoped.


Value Comparisons, Data Typing Issues
----------------------------------------------------------------------------------------------------------------------------------------------------------------

Let's see how comparison operations work in MANOOL and how the principle of strong data typing affects certain aspects of the MANOOL semantics.

### Comparing for equality #############################################################################################

First, any pair of values in MANOOL (even of different types) can be always compared for equality/inequality, two values of different types simply being deemed
unequal (even if they "look" similar):

    { {extern "manool.org.18/std/0.5/all"} in
      Out.WriteLine[2 == 2 ", " 2 <> 2 ", " "2" == "2" ", " "2" <> "2"]
      Out.WriteLine[2 == "2" ", " 2 <> "2" ", " "2" == 2 ", " "2" <> 2]
    }

Output:

    True, False, True, False
    False, True, False, True

  (In MANOOL `True` and `False` are members of a special data type _Boolean_.)

### Other operations ###################################################################################################

Any pair of integral values can be also compared for less-than as well as less-than-or-equal, greater-than, and greater-then-or-equal:

    { {extern "manool.org.18/std/0.5/all"} in
      Out.WriteLine[2 <  3 ", " 3 < 2 ", " 2 <  2] -- less-than
      Out.WriteLine[2 <= 3 ", " 2 > 3 ", " 2 >= 3] -- less-than-or-equal, greater-than, greater-then-or-equal
    }

Output:

    True, False, False
    True, False, False

On the other hand, two values of different types cannot be compared for less-than, etc.; also, the `+` operator cannot be applied to an integer and a string and
vice versa:

    {{extern "manool.org.18/std/0.5/all"} in Out.WriteLine[2 < "3"]} -- run-time error
^
    {{extern "manool.org.18/std/0.5/all"} in Out.WriteLine[2 + "3"]} -- run-time error
^
    {{extern "manool.org.18/std/0.5/all"} in Out.WriteLine["2" + 3]} -- run-time error

Output:

    Uncaught signal TypeMismatch
    ====== invocation backtrace ======
    00 at (<anonymous>) 1:56-1:62 evaluating
    ======== end of backtrace ========

  (The expression does not evaluate to any value here; such outcome is called in MANOOL _signaling_ an exception.)

And in the following example, the string `"2"` does not even "know" how to compare itself for less-than with another value, even with another string:

    {{extern "manool.org.18/std/0.5/all"} in Out.WriteLine["2" < "3"]} -- run-time error
^
    {{extern "manool.org.18/std/0.5/all"} in Out.WriteLine["2" < 3]} -- run-time error

Output:

    Uncaught signal UnrecognizedOperation
    ====== invocation backtrace ======
    00 at (<anonymous>) 1:56-1:62 evaluating
    ======== end of backtrace ========

### Type predicates ####################################################################################################

For many data types in MANOOL, there exists a type predicate, a Boolean-valued procedure that determines whether its argument belongs to the underlying type:

    { {extern "manool.org.18/std/0.5/all"} in
      Out.WriteLine[2.IsI48[] ", " "2".IsI48[]] -- is "2" an integer?
      Out.WriteLine[2.IsS8[]  ", " "2".IsS8[]]  -- is "2" a string?
    }

Output:

    True, False
    False, True


Compound Conditions
----------------------------------------------------------------------------------------------------------------------------------------------------------------

You can express complex conditions by using operators `&` (conjunction for Booleans), `|` (disjunction for Booleans), and `~` (negation for Booleans). The
operators `&` and `|` are short-circuiting (the right-hand side is unevaluated unless strictly necessary):

    {{extern "manool.org.18/std/0.5/all"} in Out.WriteLine["2".IsI48[] & "2" < 3 ", " ~"2".IsI48[] | "2" < 3]}

Output:

    False, True


More Exceptions
----------------------------------------------------------------------------------------------------------------------------------------------------------------

The MANOOL specification is precise about which exception is signaled in each particular erroneous case. We are already familiar with two of them:
`TypeMismatch` and `UnrecognizedOperation`. Let's look at other basic signals:[^a5]

First, an inappropriate number of arguments is reported by signaling `InvalidInvocation` as in

    {{extern "manool.org.18/std/0.5/all"} in Neg[2; 3]}

[^a5]: Your program in MANOOL can catch and react on any signal, and they are also used as a general control-flow mechanism.

### Arithmetic exceptions ##############################################################################################

Overflows during arithmetic operations are reported using signals:

    {{extern "manool.org.18/std/0.5/all"} in MaxI48 + 1}

Signals: `Overflow`

The same applies to division by zero and other operations near a pole of the argument (e.g. near zero for logarithms):

    {{extern "manool.org.18/std/0.5/all"} in 1 / 0}

Signals: `DivisionByZero`

In other cases `Undefined` may be signaled:

    {{extern "manool.org.18/std/0.5/all"} in 0 / 0}
^
    {{extern "manool.org.18/std/0.5/all"} in 1.Rem[0]}

Signals: `Undefined`

Quiz
----------------------------------------------------------------------------------------------------------------------------------------------------------------

Try to figure out what is going on here (you should have acquired all the clues after completing [Lesson 3](Lesson-3#start)):

    {{extern "manool.org.18/std/0.5/all"} in Out.WriteLine[(&)]}

Compilation error:

    (<anonymous>) 1:42-1:62 Error: not an R-value expression (nested in this context)

And here:

    {{extern "manool.org.18/std/0.5/all"} in Out.WriteLine[then]}

Compilation error:

    (<anonymous>) 1:42-1:58 Error: unbound keyword (nested in this context)


* * * * *
**[Continue](lesson-3 "Next Lesson")**
<aside markdown="1" class="right">
Lesson 2 of 5 -- Tutorial
=========================
+ **[Previous Lesson](lesson-1#start)**
+ **[Next Lesson](lesson-3#start)**
+ [Lesson 4](lesson-4#start)
+ [Lesson 5](lesson-5#start)
</aside>

{%endraw%}{%include page_footer.md%}
