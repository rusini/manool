---
title:   Lesson 2 &mdash; Tutorial
updated: 2020-05-04
---

{%include page_header.md%}{%raw%}


Factorial
----------------------------------------------------------------------------------------------------------------------------------------------------------------

The traditional analog of the "Hello World" program for functional languages consists of a recursive function definition to calculate the factorial of a number.
The following variation for MANOOL incorporates also some test code:

    -- Factorial -- recursive version in MANOOL-ish ("cascading") notation
    { {extern "manool.org.18/std/0.5/all"} in
    : let rec
      { Fact = -- compile-time constant binding
        { proc { N } as -- precondition: N.IsI48[] & (N >= 0)
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
          { proc { N } as -- precondition: N.IsI48[] & (N >= 0)
            { if N == 0 then 1 else
              N * Fact[N - 1]
            }
          }
        }
        in
        Out.WriteLine["Factorial of 10 = " Fact[10]]
      }
    }

As you may note, here we are using another piece of syntactic sugar -- any construct in the form  
`{`_a_<sub>0</sub> _a_<sub>1</sub> ... _a_<sub>n</sub> `{`_b_<sub>0</sub> _b_<sub>1</sub> ... _b_<sub>m</sub>`}``}` can be written as
`{`_a_<sub>0</sub> _a_<sub>1</sub> ... _a_<sub>n</sub>`:` _b_<sub>0</sub> _b_<sub>1</sub> ... _b_<sub>m</sub>`}`.

    -- Factorial -- iterative version (in MANOOL, this is probably more appropriate for factorial)
    { {extern "manool.org.18/std/0.5/all"} in
    : let
      { Fact = -- compile-time constant binding
        { proc { N } as -- precondition: N.IsI48[] & (N >= 0)
        : var { Res = 1 } in -- variable binding
        : do Res after -- return result
        : while N <> 0 do -- loop while N not equals zero
          Res = N * Res; N = N - 1
        }
      }
      in
      Out.WriteLine["Factorial of 10 is "; Fact[10]]
    }

Output:

    Factorial of 10 is 3628800


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

Any pair of integral values can be also compared for less-then as well as less-then-or-equal, greater-than, and greater-then-or-equal:

    { {extern "manool.org.18/std/0.5/all"} in
      Out.WriteLine[2 <  3 ", " 3 < 2 ", " 2 <  2] -- less-then
      Out.WriteLine[2 <= 3 ", " 2 > 3 ", " 2 >= 3] -- less-then-or-equal, greater-than, greater-then-or-equal
    }

Output:

    True, False, False
    True, False, False

On the other hand, two values of different types cannot be compared for less-then, etc.; also, the `+` operator cannot be applied to an integer and a string and
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
      Out.WriteLine[2.IsI48[] " " "2".IsI48[]] -- is "2" an integer?
      Out.WriteLine[2.IsS8[]  " " "2".IsS8[]]  -- is "2" a string?
    }

Output:

    True False
    False True

---

**Caution!!! Work in progress!!! No more prose below this line!!!**

---


## Compound Conditions
Example:
~~~
{ {extern "manool.org.18/std/0.5/all"} in
  Out.WriteLine["2".IsI48[] & "2" < 3; " "; ~"2".IsI48[] | "2" < 3]
}
~~~
Output:
~~~
False True
~~~

More Exceptions
----------------------------------------------------------------------------------------------------------------------------------------------------------------

The MANOOL specification is precise about which exception is signaled in each particular erroneous case. We are already familiar with two of them:
`TypeMismatch` and `UnrecognizedOperation`. Let's look at other basic signals, but first note that your program in MANOOL can catch and react on any signal, and
they are also used as a general control-flow mechanism.

First, an inappropriate number of arguments is reported by signaling `InvalidInvocation` as in

    {{extern "manool.org.18/std/0.5/all"} in Neg[2; 3]}

### Arithmetic Exceptions ##############################################################################################

Overflows during arithmetic operations are reported:

    {{extern "manool.org.18/std/0.5/all"} in 140737488355327/*max integer*/ + 1}

Signals: `Overflow`

~~~
{{extern "manool.org.18/std/0.5/all"} in 1 / 0}
~~~
Signals: `DivisionByZero`
~~~
{{extern "manool.org.18/std/0.5/all"} in 0 / 0}
~~~
~~~
{{extern "manool.org.18/std/0.5/all"} in 1.Rem[0]}
~~~
Signals: `Undefined`

Quiz
----------------------------------------------------------------------------------------------------------------------------------------------------------------

Try to figure out what is going on here (you should have acquired all the clues after completing the [Lesson 3](Lesson-3)):

    {{extern "manool.org.18/std/0.5/all"} in Out.WriteLine[(&)]}

Compilation error:

    (<anonymous>) 1:42-1:62 Error: not an R-value expression (nested in this context)

And here:

    {{extern "manool.org.18/std/0.5/all"} in Out.WriteLine[then]}

Compilation error:

    (<anonymous>) 1:42-1:58 Error: unbound keyword (nested in this context)


<aside markdown="1" class="right">
Lesson 2 of 5 &mdash; Tutorial
=========================
+ [Previous Lesson](lesson-1)
+ [Next Lesson](lesson-3)
+ [Lesson 4](lesson-4)
+ [Lesson 5](lesson-5)
</aside>

{%endraw%}{%include page_footer.md%}
