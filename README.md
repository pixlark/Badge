# Badge

Badge is a mostly-functional programming language inspired variously by Scheme, OCaml, Python, and Rust.

## Overview

The main workhorse of Badge is the expression; pretty much everything you want to do can be expressed as a type of expression.

Here's some examples of expressions:

```
15

nothing

if   12    % Evaluates to 6 (5 + 1)
then 5 + 1
else 2 + 3
```

One type of expression is the **scope**, denoted by a pair of curly brackets. Scopes can contain any number of **statements**, followed by an optional **expression** representing the final value of the scope itself. If no final expression is provided, the scope evaluates to `nothing`.

Here are some examples of scopes:

```
{
	let x = 12.
	println(x).
	x + 1
} % Prints 12, evaluates to 13

{
	let y = { }.
	println(y).
} % Prints `nothing`, evaluates to `nothing`
```

**Statements**, which can only exist at top-level or inside scopes, involve more imperative paradigms that any traditional programmer will be familiar with. They are terminated with a period (`.`):

```
let x = 12.

println(x).

set x = nothing.

println(x).

% outputs 12, followed by `nothing`
```

**Lambdas** are an important type of expression, and the only way to create new functions:

```
let square = lambda (x) x * x.
println(square(3)). % Outputs 9
```

Lambdas are denoted with the `lambda` keyword, followed by a list of arguments in parentheses. The body of a lambda is an expression -- any sufficiently complex lambda will likely require a scope as the body:

```
let complex = lambda (x, y) {
	let i = x + y.
	set i = i + 18 + x.
	set i = i * i.
	i
}.
println(complex(1, 2)). % Outputs 484
```

As doing true FFI is very complex, Badge instead has a very spartan FFI system. There are a series of function wrappers compiled with the language itself, and those are the only functions that can be called via FFI. The output functions are one example of this. FFI functions can be pulled into your program using the `@ffi` directive:

```
let println = @ffi[println].
println(12). % Outputs 12
```

