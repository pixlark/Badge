



let f = lambda (x) {
	return lambda () 1.
}.

print (f(1))().



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
