<img alt="MANOOL Logo" src="https://manool.org/MANOOL-Logo.png" width="64" height="64"> MANOOL v0.6.0
=====================================================================================================

**MANOOL is meant to make exploratory programming safer and faster.**

Some programming tasks are common and predictable from the project management perspective, but often, even enterprise information systems (especially in the
area of startups) involve some innovation and exploratory programming. Imagine you have such a task at hand. Whenever this happens you have two options:
  1. use an *implementation-level* programming language, such as C, C++, Java, or maybe Rust (if you want to try a more recent approach) or
  2. use a language more suitable for *throw-away* programming, such as PHP, Python, Ruby, JavaScript, or even Scheme.

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

  This happens because paying attention to those details would imply an undue cognitive burden at the early stage of development. And unlike your managers you
already knew that: a major rewrite is unavoidable, now in a "real" implementation-level language -- does this sound familiar?

While MANOOL is a general-purpose programming language, it is specifically designed to solve the above problem. It may also help you to come to a working
prototype faster and then gradually refactor your code up to a production-quality state instead of rewriting the code entirely from scratch.
