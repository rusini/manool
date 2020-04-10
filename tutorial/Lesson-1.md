---
title:   Lesson 1
updated: 2019-10-16
---

{%include page_header.md%}{%raw%}

## Hello World
~~~
-- in "applicative" notation
{{extern "manool.org.18/std/0.5/all"} in WriteLine[Out; "Hello, world!"]}
~~~
~~~
-- OOPish notation (equivalent to the above, up to Abstract Syntax Tree)
{{extern "manool.org.18/std/0.5/all"} in Out.WriteLine["Hello, world!"]}
~~~
~~~
-- LISPish notation (ditto)
{{extern "manool.org.18/std/0.5/all"} in {WriteLine Out "Hello, world!"}}
~~~
Output:
~~~
Hello, world!
~~~
### Code Formatting
~~~
-- most recommended formatting for multi-line expressions
{ {extern "manool.org.18/std/0.5/all"} in
  WriteLine[Out; "Hello, world!"]
}
~~~
### Comments
~~~
-- this is a comment
{{extern "manool.org.18/std/0.5/all"} in Out.WriteLine[/* Out; */ "Hello, world!"]}
~~~

## Assembling Phrases from Fragments
Example:
~~~
-- evaluation of multiple expressions in a row
{ {extern "manool.org.18/std/0.5/all"} in
  Out.Write["Hello"]; Out.Write[", "]; Out.Write["world"]; Out.Write["!"]
  Out.WriteLine[]
  -- multiple arguments to Write/WriteLine
  Out.WriteLine["Hello"; ", "; "world"; "!"]; -- ";"-delimiters are optional
  Out.WriteLine["Hello" ", "; "world" "!"]    -- everywhere where they are allowed
}
~~~
Output:
~~~
Hello, world!
Hello, world!
Hello, world!
~~~
### Concatenation, Infix Operators
Example:
~~~
-- string concatenation
{ {extern "manool.org.18/std/0.5/all"} in
  -- conventional (infix) notation
  Out.WriteLine["Hello" + "," + " " + "world!"]
  -- applicative (prefix) notation (equivalent up to AST)
  Out.WriteLine[(+)[(+)[(+)["Hello"; ", "]; "world"]; "!"]]
  -- LISPish (prefix) notation (ditto)
  Out.WriteLine[{(+) {(+) {(+) "Hello" ", "} "world"} "!"}]
  -- OOPish (infix) notation (ditto)
  Out.WriteLine["Hello".(+)[", "].(+)["world"].(+)["!"]]
}
~~~
Output:
~~~
Hello, world!
Hello, world!
Hello, world!
Hello, world!
~~~

## Simple Arithmetic
Example:
~~~
-- performing some integer arithmetic and displaying results
{ {extern "manool.org.18/std/0.5/all"} in
  -- outputting piece-wise
  Out.WriteLine["2 + 3 = " 2 + 3; ", "; "5 - 2 = " 5 - 2]
  -- explicit conversion to string and concatenation
  Out.WriteLine["2 + 3 = " + Str[2 + 3] + ", " + "5 - 2 = " + Str[5 - 2]]
}
~~~
Output:
~~~
2 + 3 = 5, 5 - 2 = 3
2 + 3 = 5, 5 - 2 = 3
~~~
### Multiplication, Operator Precedence and Associativity
Example:
~~~
{ {extern "manool.org.18/std/0.5/all"} in
  -- precedence (binding strength) and associativity -----
  Out.WriteLine[
  "2 + 3 * 4 - 5 = " 2 + 3 * 4 - 5; ", "; "(2 + (3 * 4)) - 5 = " (2 + (3 * 4)) - 5]
  -- override with explicit grouping -----
  Out.WriteLine["(2 + 3) * (4 - 5) = " (2 + 3) * (4 - 5)]
}
~~~
Output:
~~~
2 + 3 * 4 - 5 = 9, (2 + (3 * 4)) - 5 = 9
(2 + 3) * (4 - 5) = -5
~~~
### Unary Minus
Example:
~~~
{ {extern "manool.org.18/std/0.5/all"} in
  Out.WriteLine["Neg[2] = " Neg[2]; ", "; "5 + ~2 = " 5 + ~2]
}
~~~
Output:
~~~
Neg[2] = -2, 5 + ~2 = 3
~~~
### Division
Example:
~~~
-- integer division and related operations
{ {extern "manool.org.18/std/0.5/all"} in
  -- heading -----
  Out.WriteLine["     8,3   -8,3   8,-3  -8,-3"]
  -- integer division (truncating) -----
  Out.WriteLine[" /    " 8  /  3  "     "  ~8   /  3  "     " 8  /  ~3  "     "  ~8   /  ~3]
  -- remainder from integer division -----
  Out.WriteLine["Rem   " 8.Rem[3] "     " (~8).Rem[3] "     " 8.Rem[~3] "     " (~8).Rem[~3]]
  Out.WriteLine["Rem " 8.Rem[3].Str["2d"] " " (~8).Rem[3].Str["2d"] " " 8.Rem[~3].Str["2d"] " " (~8).Rem[~3].Str["2d"]]
  -- flooring division -----
  Out.WriteLine["Div   " 8.Div[3] "     " (~8).Div[3] "     " 8.Div[~3] "     " (~8).Div[~3]]
  -- modulo (remainder from flooring division) -----
  Out.WriteLine["Mod   " 8.Mod[3] "     " (~8).Mod[3] "     " 8.Mod[~3] "     " (~8).Mod[~3]]
}
~~~
Output:
~~~
     8,3   -8,3   8,-3  -8,-3
 /    2     -2     -2     2
Rem   2     -2     2     -2
Div   2     -3     -3     2
Mod   2     1     -1     -2
~~~

---

Try to figure out what is going on here (answer in Lesson 3):
~~~
{ {extern "manool.org.18/std/0.5/all"} in
  -- what do Write/WriteLine return? -----
  Out.WriteLine[
  Out.Write["Hello"]; " "; Out.Write[", "]; " ";
  Out.Write["world"]; " "; Out.Write["!"]; " "; Out.WriteLine[]]
  -- what are Out, WriteLine, (+)?.. printing "unprintable"? -----
  Out.WriteLine[
  Out; " "; WriteLine; " "; (+); " "; (~); " "; Foo; " "; Bar]
}
~~~
Output:
~~~
Hello, world!
Nil Nil Nil Nil Nil
value/object WriteLine + ~ Foo Bar
~~~

{%endraw%}{%include page_footer.md%}
