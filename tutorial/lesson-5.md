---
title:   Lesson 5 -- Tutorial
updated: 2020-05-16
---

<aside markdown="1" class="right">
Lesson 5 of 5 -- Tutorial
=========================
+ [Lesson 1](lesson-1#start)
+ [Lesson 2](lesson-2#start)
+ [Lesson 3](lesson-3#start)
+ **[Previous Lesson](lesson-4#start)**
</aside>

{%include page_header.md%}{%raw%}


Rational Numbers ADT
----------------------------------------------------------------------------------------------------------------------------------------------------------------

---

**Caution!!! Work in progress!!!**

---

Example:

    { {extern "manool.org.18/std/0.5/all"} in
    : do
      { Rationals in -- demo
      : var { A; B } in
        A = Rat[1; 2]
        B = Rat[3; 4]
        Out.WriteLine["A = "; A]
        Out.WriteLine["B = "; B]
      : for { Op = {array of (+); (-); (*); (/)} } do
        Out.WriteLine["A "; Op; " B = "; A.Op[B]]
      }
      where
      Rationals =
      { scope { extern } in
      : {extern "manool.org.18/std/0.5/all"} in
      : let { _Num; _Den } in
      : let { IsRat = {{object utils _Num; _Den} in IsInst} } in -- classification
      : let
        { Gcd = -- Greatest Common Divisor of {Num, Den} - snippet of code
          { if Num == 0 then 1 else
          : var { A = Abs[Num]; B = Den } in
          : do A after
          : while A <> B do: if A > B then A = A - B else B = B - A -- Euclidean algorithm
          }'
        }
        in
      : let rec
        { Rat = -- construction
          { proc { Num; Den } as
          : var { Gcd = Gcd% } in
          : object { _Num = Num / Gcd; _Den = Den / Gcd } with
            -- extractors
            Num = {proc {A} as A[_Num]@}
            Den = {proc {A} as A[_Den]@}
            -- arithmetic operations
            (+) =
            { proc { A; B } as
            : unless IsRat[B] signal TypeMismatch else
              Rat[Num[A] * Den[B] + Num[B] * Den[A]; Den[A] * Den[B]]
            }
            (-) =
            { proc { A; B } as
            : unless IsRat[B] signal TypeMismatch else
              Rat[Num[A] * Den[B] - Num[B] * Den[A]; Den[A] * Den[B]]
            }
            (*) =
            { proc { A; B } as
            : unless IsRat[B] signal TypeMismatch else
              Rat[Num[A] * Num[B]; Den[A] * Den[B]]
            }
            (/) =
            { proc { A; B } as
            : unless IsRat[B] signal TypeMismatch else
            : unless Num[B] <> 0 signal DivisionByZero else
            : if Num[B] > 0 then
              Rat[ Num[A] * Den[B]; Den[A] *  Num[B]]
              else
              Rat[~Num[A] * Den[B]; Den[A] * ~Num[B]]
            }
            (~) = Neg -- unary minus (arithmetic negation)
            Neg =
            { proc { A } as
              Rat[~Num[A]; Den[A]]
            }
            -- comparison
            (==) =
            { proc { A; B } as
               IsRat[B] & (Num[A] == Num[B]) & (Den[A] == Den[B])
            }
            (<>) =
            { proc { A; B } as
              ~IsRat[B] | (Num[A] <> Num[B]) | (Den[A] <> Den[B])
            }
            (<)  =
            { proc { A; B } as
            : unless IsRat[B] signal TypeMismatch else
              Num[A] * Den[B] <  Num[B] * Den[A]
            }
            (<=) =
            { proc { A; B } as
            : unless IsRat[B] signal TypeMismatch else
              Num[A] * Den[B] <= Num[B] * Den[A]
            }
            (>)  =
            { proc { A; B } as
            : unless IsRat[B] signal TypeMismatch else
              Num[A] * Den[B] >  Num[B] * Den[A]
            }
            (>=) =
            { proc { A; B } as
            : unless IsRat[B] signal TypeMismatch else
              Num[A] * Den[B] >= Num[B] * Den[A]
            }
            Order =
            { proc { A; B } as
            : unless IsRat[B] signal TypeMismatch else
              Order[Num[A] * Den[B]; Num[B] * Den[A]]
            }
            -- conversions and cloning
            Str   = {proc {A} as Str[Num[A]] + "/" + Str[Den[A]]}
            Int   = {proc {A} as Num[A] / Den[A]}
            Clone = {proc {A} as Rat[Num[A]; Den[A]]}
          } -- proc
        }
        in
      : ensure Rat[1; 2] + Rat[ 3; 4] == Rat[ 5; 4] in -- unit tests
      : ensure Rat[1; 2] - Rat[ 3; 4] == Rat[~1; 4] in
      : ensure Rat[1; 2] * Rat[ 3; 4] == Rat[ 3; 8] in
      : ensure Rat[1; 2] / Rat[ 3; 4] == Rat[ 2; 3] in
      : ensure Rat[1; 2] / Rat[~3; 4] == Rat[~2; 3] in
      : ensure Rat[1; 2] < Rat[ 3; 4] in
      : export
        Rat = -- construction
        { proc { Num; Den } as
        : unless IsI48[Num] & IsI48[Den] signal TypeMismatch else
        : unless Den <> 0 signal {if Num <> 0 then DivisionByZero else Undefined} else
        : if Den > 0 then Rat[Num; Den] else Rat[~Num; ~Den]
        }
        IsRat = IsRat -- classification
      } -- scope
    }

Output:

    A = 1/2
    B = 3/4
    A + B = 5/4
    A - B = -1/4
    A * B = 3/8
    A / B = 2/3


<aside markdown="1" class="right">
Lesson 5 of 5 -- Tutorial
=========================
+ [Lesson 1](lesson-1#start)
+ [Lesson 2](lesson-2#start)
+ [Lesson 3](lesson-3#start)
+ **[Previous Lesson](lesson-4#start)**
</aside>

{%endraw%}{%include page_footer.md%}
