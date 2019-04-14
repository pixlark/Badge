

let function = lambda () {
	% This should get collected
	let inner = lambda (x) { print x. }.
	inner(2).
}.

let x = function().
print x.


[-
let simple_closure = lambda (x) lambda () x.
print simple_closure(111)().

let curry = lambda (x) (lambda (y) (lambda (z) (z + y + x))).
print curry(1)(2)(4).-]


[-
let f = lambda (x) {
	return lambda () x.
}.

print (f(100))().-]


[-
let f = lambda (x) x.
print f(nothing).-]


[-
let add = lambda (a, b) a + b.
print add(1, 2).-]



[-
print {
	let x = 1.
	x
}.

print x.-]

[-
let function = lambda (x) {
	print x.
	print x + 1.
	return x + 1.
}.

let x = 1.
set x = function(x).
set x = function(x).
set x = function(x).
set x = function(x).-]

%% This shouldn't work
%lambda (x) { print x; }(1);
%% This should work
%(lambda (x) { print x. })(1).

%% TODO: Closures
[-
let add = lambda (x) {
	return lambda (y) {
		return x + y;
	};
};

let increment = add(1);
print increment(2);
-]
