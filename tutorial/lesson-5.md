---
title:   Lesson 5 -- Tutorial
updated: 2020-06-01
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

In this section we'll discuss a rather long piece of code -- taking as an example rational numbers, we'll see how to define your own data types in MANOOL.

This involves studying several orthogonal mechanisms and constructs:
  * modules,
  * uninterned (unique) symbols,
  * concrete object constructors (along with `utils` modules), and
  * exception signaling.

  Some auxiliary constructs will be exposed (and explained) too:
  * `{do ... where ...}`,
  * `{scope {...} in ...}`,
  * `(...)%`,
  * `{unless ... signal ... else ...}`, and
  * `{ensure ... in ...}`.

<aside markdown="1">

The ability to specify precisely all properties of a custom data type is both a natural and essential abstraction mechanism. In MANOOL, these properties
include
  * a set of polymorphic operations defined for a data type and
  * a concrete representation.

  We call such types abstract data types (ADTs) because you have full control over direct access to the concrete representation of values (or objects) of ADTs
(and you might want to restrict somehow such access from outside of the type definition).

Full-blown ADTs are often (but not always) an element of object-oriented programming (OOP). Although MANOOL stands for "**MA**NOOL is *Not* an Object-Oriented
Language", it is no more than a pun on certain marketing trends. MANOOL in fact supports the following mechanisms typical for OOP:
  * data encapsulation (hiding data representation details) and
  * dynamic dispatch (via polymorphic operations, which is originally a kind of dynamic overloading mechanism).

  To a lesser degree, MANOOL supports inheritance in a traditional sense (and this is a deliberate design choice).

MANOOL does not include any class definition mechanism, which make it more of a prototype-based OOP language and makes it easy to construct new kinds objects
on-the-fly without any special boiler-plate code.

</aside>

The following example constitutes a working program with a complete "rational number" data type definition and some test code (for ease of reference, line
numbers are added as valid comments to the listing):

    /*001*/  { {extern "manool.org.18/std/0.6/all"} in
    /*002*/  : do
    /*003*/    { Rationals in -- Test code
    /*004*/    : var { A; B } in
    /*005*/      A = Rat[1; 2]$ -- one half rational constant
    /*006*/      B = Rat[3; 4]$ -- three fourth rational constant
    /*007*/      Out.WriteLine["A = " A] -- displaying rationals
    /*008*/      Out.WriteLine["B = " B]
    /*009*/    : for { Op = {array of (+) (-) (*) (/)} } do
    /*010*/      Out.WriteLine["A " Op " B = " A.Op[B]] -- rational arithmetic
    /*011*/    }
    /*012*/    where
    /*013*/    Rationals =
    /*014*/    { scope { extern } in
    /*015*/    : {extern "manool.org.18/std/0.6/all"} in
    /*016*/    : let { _Num; _Den } in
    /*017*/    : let { IsRat = {{object utils _Num; _Den} in IsInst} } in -- classification
    /*018*/    : let
    /*019*/      { Gcd = -- Greatest Common Divisor of {Num, Den} - Snippet of code
    /*020*/        { if Num == 0 then 1 else
    /*021*/        : var { A = Abs[Num]; B = Den } in
    /*022*/        : do A after
    /*023*/        : while A <> B do: if A > B then A = A - B else B = B - A -- Euclidean algorithm
    /*024*/        }'
    /*025*/      }
    /*026*/      in
    /*027*/    : let rec
    /*028*/      { Rat = -- Construction
    /*029*/        { proc { Num; Den } as
    /*030*/        : var { Gcd = Gcd% } in
    /*031*/        : object { _Num = Num / Gcd; _Den = Den / Gcd } with
    /*032*/          -- Extractors
    /*033*/          Num = {proc {A} as A[_Num]@}
    /*034*/          Den = {proc {A} as A[_Den]@}
    /*035*/          -- Arithmetic operations
    /*036*/          (+) =
    /*037*/          { proc { A; B } as
    /*038*/          : unless IsRat[B] signal TypeMismatch else
    /*039*/            Rat[Num[A] * Den[B] + Num[B] * Den[A]; Den[A] * Den[B]]
    /*040*/          }
    /*041*/          (-) =
    /*042*/          { proc { A; B } as
    /*043*/          : unless IsRat[B] signal TypeMismatch else
    /*044*/            Rat[Num[A] * Den[B] - Num[B] * Den[A]; Den[A] * Den[B]]
    /*045*/          }
    /*046*/          (*) =
    /*047*/          { proc { A; B } as
    /*048*/          : unless IsRat[B] signal TypeMismatch else
    /*049*/            Rat[Num[A] * Num[B]; Den[A] * Den[B]]
    /*050*/          }
    /*051*/          (/) =
    /*052*/          { proc { A; B } as
    /*053*/          : unless IsRat[B] signal TypeMismatch else
    /*054*/          : unless Num[B] <> 0 signal DivisionByZero else
    /*055*/          : if Num[B] > 0 then
    /*056*/            Rat[ Num[A] * Den[B]; Den[A] *  Num[B]]
    /*057*/            else
    /*058*/            Rat[~Num[A] * Den[B]; Den[A] * ~Num[B]]
    /*059*/          }
    /*060*/          (~) = Neg -- unary minus (arithmetic negation)
    /*061*/          Neg =
    /*062*/          { proc { A } as
    /*063*/            Rat[~Num[A]; Den[A]]
    /*064*/          }
    /*065*/          -- Comparison
    /*066*/          (==) =
    /*067*/          { proc { A; B } as
    /*068*/             IsRat[B] & (Num[A] == Num[B]) & (Den[A] == Den[B])
    /*069*/          }
    /*070*/          (<>) =
    /*071*/          { proc { A; B } as
    /*072*/            ~IsRat[B] | (Num[A] <> Num[B]) | (Den[A] <> Den[B])
    /*073*/          }
    /*074*/          (<)  =
    /*075*/          { proc { A; B } as
    /*076*/          : unless IsRat[B] signal TypeMismatch else
    /*077*/            Num[A] * Den[B] <  Num[B] * Den[A]
    /*078*/          }
    /*079*/          (<=) =
    /*080*/          { proc { A; B } as
    /*081*/          : unless IsRat[B] signal TypeMismatch else
    /*082*/            Num[A] * Den[B] <= Num[B] * Den[A]
    /*083*/          }
    /*084*/          (>)  =
    /*085*/          { proc { A; B } as
    /*086*/          : unless IsRat[B] signal TypeMismatch else
    /*087*/            Num[A] * Den[B] >  Num[B] * Den[A]
    /*088*/          }
    /*089*/          (>=) =
    /*090*/          { proc { A; B } as
    /*091*/          : unless IsRat[B] signal TypeMismatch else
    /*092*/            Num[A] * Den[B] >= Num[B] * Den[A]
    /*093*/          }
    /*094*/          Order =
    /*095*/          { proc { A; B } as
    /*096*/          : unless IsRat[B] signal TypeMismatch else
    /*097*/            Order[Num[A] * Den[B]; Num[B] * Den[A]]
    /*098*/          }
    /*099*/          -- Conversions and cloning
    /*100*/          Str   = {proc {A} as Str[Num[A]] + "/" + Str[Den[A]]}
    /*101*/          Int   = {proc {A} as Num[A] / Den[A]}
    /*102*/          Clone = {proc {A} as Rat[Num[A]; Den[A]]}
    /*103*/        } -- proc
    /*104*/      }
    /*105*/      in
    /*106*/    : ensure Rat[1; 2] + Rat[ 3; 4] == Rat[ 5; 4] in -- unit tests
    /*107*/    : ensure Rat[1; 2] - Rat[ 3; 4] == Rat[~1; 4] in
    /*108*/    : ensure Rat[1; 2] * Rat[ 3; 4] == Rat[ 3; 8] in
    /*109*/    : ensure Rat[1; 2] / Rat[ 3; 4] == Rat[ 2; 3] in
    /*110*/    : ensure Rat[1; 2] / Rat[~3; 4] == Rat[~2; 3] in
    /*111*/    : ensure Rat[1; 2] < Rat[ 3; 4] in
    /*112*/    : export
    /*113*/      Rat = -- Construction
    /*114*/      { proc { Num; Den } as
    /*115*/      : unless IsI48[Num] & IsI48[Den] signal TypeMismatch else
    /*116*/      : unless Den <> 0 signal {if Num <> 0 then DivisionByZero else Undefined} else
    /*117*/      : if Den > 0 then Rat[Num; Den] else Rat[~Num; ~Den]
    /*118*/      }
    /*119*/      IsRat = IsRat -- classification
    /*120*/    } -- scope
    /*121*/  }

Output:

    A = 1/2
    B = 3/4
    A + B = 5/4
    A - B = -1/4
    A * B = 3/8
    A / B = 2/3

---

**Caution!!! Work in progress!!!**

---


<aside markdown="1" class="right">
Lesson 5 of 5 -- Tutorial
=========================
+ [Lesson 1](lesson-1#start)
+ [Lesson 2](lesson-2#start)
+ [Lesson 3](lesson-3#start)
+ **[Previous Lesson](lesson-4#start)**
</aside>

{%endraw%}{%include page_footer.md%}
