# Badge

Badge is a primarily functional programming language inspired variously by Scheme, OCaml, Python, and Rust.

*If you want to see something a little more complex, check out*  [this test file](https://github.com/pixlark/Badge/blob/master/tests/samples/linked-list.bdg) *which implements a simple linked in Badge.*

## The Language

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

Our basic **types** are few: integers (`let x = 1`) and strings (`let s = "string"`).

**Statements**, which can only exist at top-level or inside scopes, involve more imperative paradigms that any traditional programmer will be familiar with. They are terminated with a period (`.`):

```
let x = 12.

println(x).

set x = nothing.

println(x).

% outputs 12, followed by `nothing`
```

**If expressions** are our branching control structure. They take the following form:

```
if   <condition>
then <consequence>
elif <condition
then <consequence>
...
else <consequence>
```

An if expression is an *expression* because it will evaluate to the evaluation of whichever path is chosen.

The conditions of an if expression can evaluate to anything, as we take the Lisp route of having everything except for the `nothing` value mean true, and the `nothing` value mean false:

```
println(1 == 0). % 'nothing'
println(1 == 1). % Could be anything, in practice the default truth value is '1'
```

The standard library defines two constants, `true` and `false`, which are equivalent to `1` and `nothing`, respectively.

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

The **this** keyword is used inside of lambda expressions to indicate recursion. This is better practice than using the name of the function directly, although that is also possible:

```
let factorial = lambda (n) if   n == 0
                           then 1
                           else n * this(n - 1).
```

**Loop expressions** are a simple looping control structure, for when recursion is a bit overkill. In general, however, recursion is the preferred way to do things. Loop expressions take the following simple form:

```
loop <expr>
```

`<expr>` will repeat infinitely, until a `break` statement is run from within. A break statement needs to have some value passed to it, which is what the loop expression itself evaluates to. For example:

```
println(loop {
	break 15.
}). % This will print '15'
```

**Constructors and Objects** are a simple way to store information together. The `@struct` directive is used to create new constructors:

```
let Point = @struct[x, y].
```

Constructors are used to instantiate objects:

```
let p = Point(1, 2).
```

And finally, the `'` operator is used to access the fields of an object:

```
println(p'x).
println(p'y).
```

**Imports and Exports** are the way you split code up among multiple files.

The `@import` directive takes a path to some file, in relation to the file you're currently running, and runs it. Anything exported by that file gets pulled into your global namespace:

```
@import["print_routines.bdg"].

println("Hello!").
```

The `@export` directive defines which names from your file you want to be exported:

```
@export[println].

let println = @builtin[println].
```

To run any program, you need to point to the **standard library**. This is done by setting the environment variable `BADGE_STDLIB_PATH` to the path of the library before running the interpreter.

To use something from the standard library, use an `@import` directive using a symbol rather than a string:

```
@import[prelude].

println("Hello, world!").
```

**Built-in functions** that can't be expressed in terms of pure Badge are imported using the `@builtin` directive as so:

```
let println = @builtin[println].
println(12). % Outputs 12
```

Although this is done for you most of the time via the standard library.

Finally, we have two types of **comments:**

```
% Line comment
[-
  block comment
-]
```
