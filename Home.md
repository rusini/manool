---
# Home.md
permalink: /
title:     Introduction
updated:   2020-08-18
---

<aside markdown="1" class="right">

*My name is Alex Protasov (AKA [rusini](about "About @rusini")) and I have designed and implemented a new programming language called MANOOL (with no external
funding or endorsement).*

<figure><img src="Manul-Cat.jpeg" width="204" height="272" alt="Pallas's cat" title="Pallas's cat"></figure>

The name "MANOOL" is a [recursive acronym] that stands for "**MA**NOOL is _Not_ an [Object-Oriented Language]!"; it is just a [pun] on the renowned marketing
hype of the past. "MANOOL" should be pronounced as "manul" ([the Pallas's cat]).

[recursive acronym]:        https://en.wikipedia.org/wiki/Recursive_acronym        "Wikipedia: Recursive acronym"
[Object-Oriented Language]: https://en.wikipedia.org/wiki/Object-oriented_language "Wikipedia: Object-oriented language"
[pun]:                      https://en.wikipedia.org/wiki/Pun                      "Wikipedia: Pun"
[the Pallas's cat]:         https://en.wikipedia.org/wiki/Pallas%27s_cat           "Wikipedia: Pallas's cat"

</aside>

{%include page_header.md%}{%raw%}


**MANOOL is meant to make exploratory programming safer and faster.**

Some programming tasks are common and predictable from the project management perspective, but often, even enterprise information systems (especially in the
area of startups) involve some innovation and [exploratory programming]. Imagine you have such task at hand. Whenever this happens you have two options:
  1. use an *implementation-level* programming language, such as C, C++, Java, or maybe Rust (if you want to try a more recent approach) or
  2. use a language more suitable for *throw-away* programming, such as PHP, Python, Ruby, JavaScript, or even Scheme.

[exploratory programming]: https://en.wikipedia.org/wiki/Exploratory_programming "Wikipedia: Exploratory programming"

In the former case, you eventually get stuck with your coding -- trying to conceive some poorly understood algorithms, deciding which data types to use and how
to get around seemingly arbitrary constraints for composite data types, devising resource management policies, and dealing with confusing program logic.

Then you resort to the second option, in which case you also have to conceive poorly understood algorithms, deal with confusing program logic, and occasionally
think about how to circumvent composite data type constraints, but most probably you end up familiarized yourself with the problem domain and come to a working
prototype.

You show your solution (which mostly looks nice) to the managers, and suddenly they react: "OK, let's clear up the bugs; tomorrow we deploy it in production!".
Then disaster falls on you; after some time of production use, it turns out that
  * your code is not scalable to a grown user base and hence larger workload, or the solution is simply slow according to your end users,
  * your code has mysterious and hard to localize bugs, and of course
  * the program logic itself still looks confusing and complex.

  This happens because paying attention to those details would imply undue cognitive burden at the early stage of development. And unlike your managers you
already knew that: a major rewrite is unavoidable, now in a "real" implementation-level language -- does this sound familiar?

While MANOOL is a general-purpose programming language, it is specifically designed to solve the above problem. It may also help you to come to a working
prototype faster and then gradually refactor your code up to a production-quality state instead of rewriting the code entirely from scratch.

<aside markdown="1">

What MANOOL offers in order to achieve this that the present-day alternatives do not?

Consider the following language/feature matrix where the level of support goes on the scale from 1 to 5 points:

<div markdown="1" class="scroll">

  Language  |F. **1**|F. **2**|F. **3**|F. **4**|F. **5**|F. **6**|F. **7**
------------|--------|--------|--------|--------|--------|--------|--------
 PHP        | +++    | +++    | +++    | +      | ++++   | +++++  | +++
 Python     | ++     | ++     | ++++   | ++     | +++++  | +++    | +++
 Ruby       | +++    | +      | +++    | ++     | +++++  | +      | +++
 JavaScript | +++    | +      | +++    | +      | +++++  | +      | +++
 Scheme     | ++++   | +      | ++     | +++    | +++++  | +      | +++
 MANOOL     | +++++  | +++++  | +++++  | +++++  | +++++  | +++++  | +++++

</div>

**Features (F.n above):**
1. Syntax uniformity and scalability
2. Value (i.e., copy-on-write) storage semantics (by default)
3. High-level composites (sets, mappings, sequences, etc.) and comprehensions of them, without ad-hoc constraints
4. Strong typing and control over semantics of basic operations
5. Control over run time scalability (asymptotic complexity)
6. Deterministic (reference-counting based) memory/resource reclamation
7. Run-time recoverability (collaborative)

</aside>


{%endraw%}{%include page_footer.md%}
