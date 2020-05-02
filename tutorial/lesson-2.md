---
title:   Lesson 2
updated: 2019-10-16
---

{%include page_header.md%}{%raw%}

## Factorial
Examples:
~~~
-- recursive version, MANOOLish "cascading" notation
{ {extern "manool.org.18/std/0.5/all"} in
: let rec
  { Fact = -- compile-time constant binding
    { proc { N } as -- precondition: N.IsI48[] & (N >= 0)
    : if N == 0 then 1 else
      N * Fact[N - 1]
    }
  }
  in
  Out.WriteLine["Factorial of 10 is "; Fact[10]]
}
~~~
~~~
-- conventional notation (equivalent to the above up to AST)
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
    Out.WriteLine["Factorial of 10 is "; Fact[10]]
  }
}
~~~
~~~
-- iterative version (in MANOOL, probably more appropriate in this particular case)
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
~~~
Output:
~~~
Factorial of 10 is 3628800
~~~

## Data Typing Issues
Example:
~~~
{{extern "manool.org.18/std/0.5/all"} in Out.WriteLine["2" == 2; " "; 2 == "2"]}
~~~
Output:
~~~
False False
~~~
Examples:
~~~
{{extern "manool.org.18/std/0.5/all"} in Out.WriteLine["2" < 3]} -- run-time error
~~~
~~~
{{extern "manool.org.18/std/0.5/all"} in Out.WriteLine["2" + 3]} -- run-time error
~~~
Output:
~~~
Uncaught signal UnrecognizedOperation
====== invocation backtrace ======
00 at (<anonymous>) 1:56-1:62 evaluating
======== end of backtrace ========
~~~
Examples:
~~~
{{extern "manool.org.18/std/0.5/all"} in Out.WriteLine[2 < "3"]} -- run-time error
~~~
~~~
{{extern "manool.org.18/std/0.5/all"} in Out.WriteLine[2 + "3"]} -- run-time error
~~~
Output:
~~~
Uncaught signal TypeMismatch
====== invocation backtrace ======
00 at (<anonymous>) 1:56-1:62 evaluating
======== end of backtrace ========
~~~
Example:
~~~
{ {extern "manool.org.18/std/0.5/all"} in
  Out.WriteLine[2.IsI48[]; " "; "2".IsI48[]] -- is an integer?
  Out.WriteLine[2.IsS8[];  " "; "2".IsS8[]]  -- is a string?
}
~~~
Output:
~~~
True False
False True
~~~

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

## More Exceptions
~~~
{{extern "manool.org.18/std/0.5/all"} in Neg[2; 3]}
~~~
Signals: `InvalidInvocation`
### Arithmetic Exceptions
~~~
{{extern "manool.org.18/std/0.5/all"} in 140737488355327/*max integer*/ + 1}
~~~
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

-----

Try to figure out what is going on here (answer in Lesson 3):
~~~
{{extern "manool.org.18/std/0.5/all"} in Out.WriteLine[(&)]}
~~~
~~~
{{extern "manool.org.18/std/0.5/all"} in Out.WriteLine[then]}
~~~
Output:
~~~
(<anonymous>) 1:42-1:62 Error: not an R-value expression (nested in this context)
~~~
~~~
(<anonymous>) 1:42-1:58 Error: unbound keyword (nested in this context)
~~~

{%endraw%}{%include page_footer.md%}
