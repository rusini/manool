---
title:   Lesson 3
updated: 2019-10-16
---

{%include page_header.md%}{%raw%}

## Symbol (Data-Type)
Example:
~~~
{ {extern "manool.org.18/std/0.5/all"} in
  Out.WriteLine[Foo; " "; Foo == "Foo"; " "; Foo == MakeSym["Foo"]; " "; Foo.IsSym[]]
}
~~~
Output:
~~~
Foo False True True
~~~
Example:
~~~
{ {extern "manool.org.18/std/0.5/all"} in
  Out.WriteLine[Foo'; " "; (&)'; " "; then'] -- (') is a postfix operator
}
~~~
Output:
~~~
Foo & then
~~~

## Working with Fractions
Example:
~~~
{ {extern "manool.org.18/std/0.5/all"} in
  Out.WriteLine[{do Out.WriteLine["One"]; 2 + 3}]  -- do - explicit sequencing
  Out.WriteLine[{do Out.WriteLine["Two"]; 5 - 2}$] -- $  - compile-time evaluation
}
~~~
Output:
~~~
Two
One
5
3
~~~
Example:
~~~
{ {extern "manool.org.18/std/0.5/all"} in
  -- scientific-oriented arithmetic (base-2 internal representation) -----
  -- precise representation
  Out.WriteLine["one quarter (F64)  = "; F64[".25"]$;  " = "; (F64[1] / F64[4])$]
  -- approximate representation
  Out.WriteLine["one tenth   (F64)  = "; F64["1e-1"]$; " = "; (F64[1] / F64[10])$]
  -- periodic fraction (if represented in decimal form)
  Out.WriteLine["one third   (F64)  = "; (F64[1] / F64[3])$]
  Out.WriteLine["one third   (F32)  = "; (F32[1] / F32[3])$]
  -- human-oriented arithmetic (base-10 internal representation) -----
  Out.WriteLine["one tenth   (D64)  = "; D64["1e-1"]$; " = "; D64[".10"]$]
  Out.WriteLine["one third   (D64)  = "; (D64[1] / D64[3])$]
  Out.WriteLine["one third   (D128) = "; (D128[1] / D128[3])$]
}
~~~
Output:
~~~
one quarter (F64)  = 2.5000000000000000e-01 = 2.5000000000000000e-01
one tenth   (F64)  = 1.0000000000000001e-01 = 1.0000000000000001e-01
one third   (F64)  = 3.3333333333333331e-01
one third   (F32)  = 3.33333343e-01
one tenth   (D64)  = 0.1 = 0.10
one third   (D64)  = 0.3333333333333333
one third   (D128) = 0.3333333333333333333333333333333333
~~~
### Decimal Rounding
Example:
~~~
{ {extern "manool.org.18/std/0.5/all"} in
  Out.WriteLine["Banker's rounding: ";
  D64["2.000000000000021"]$ / D64[2]$; ", "; D64["2.000000000000031"]$ / D64[2]$;
  " "; D64[".25"]$.Quantize[D64[".0"]$]; ", "; D64[".35"]$.Quantize[D64[".0"]$]]
  Out.WriteLine["Common rounding:   ";
  C64["2.000000000000021"]$ / C64[2]$; ", "; C64["2.000000000000031"]$ / C64[2]$;
  " "; C64[".25"]$.Quantize[C64[".0"]$]; ", "; C64[".35"]$.Quantize[C64[".0"]$]]
}
~~~
Output:
~~~
Banker's rounding: 1.000000000000010, 1.000000000000016, 0.2, 0.4
Common rounding:   1.000000000000011, 1.000000000000016, 0.3, 0.4
~~~
### Signed Zeros
Example:
~~~
{ {extern "manool.org.18/std/0.5/all"} in
  -- signed zeros supported
  Out.WriteLine[F64["1e-300"]$ / F64["1e300"]$; ", "; ~F64["1e-300"]$ / F64["1e300"]$]
  Out.WriteLine[F64["+0"]$; ", "; F64["-0"]$]
  Out.WriteLine[F64["+0"]$ == F64["-0"]$]
  Out.WriteLine[Atan[F64[0]$; F64["+0"]$]]
  Out.WriteLine[Atan[F64[0]$; F64["-0"]$]]
  -- signed zeros unsupported
  Out.WriteLine[D64["1e-300"]$ / D64["1e300"]$; ", "; ~D64["1e-300"]$ / D64["1e300"]$]
  Out.WriteLine[D64["+0"]$; ", "; D64["-0"]$]
}
~~~
Output:
~~~
0.0000000000000000e+00, -0.0000000000000000e+00
0.0000000000000000e+00, -0.0000000000000000e+00
True
0.0000000000000000e+00
3.1415926535897931e+00
0e-398, 0e-398
0, 0
~~~

-----

Try to figure out what is going on here:
~~~
{ {extern "manool.org.18/std/0.5/all"} in
  Out.WriteLine[Out.WriteLine["Hello, world!"]']
}
~~~
Output:
~~~
value/object
~~~

{%endraw%}{%include page_footer.md%}
