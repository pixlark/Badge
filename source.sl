


let f = lambda (x) {
	lambda (y) {
		lambda () { x + y }
	}
}.

let g = f(1).
let h = g(2).
print h().




[-
let f = lambda () {
	let x = 10.
	lambda () { print x. }
}.

let g = f().
g().-]












[-
{
	let fact = lambda (x) if x == 0
			   		  	  then 1
						  else x * fact(x - 1).
	print fact(1).
}.-]

%% Factorial
[-
let fact =
  lambda (x) if x == 0
  		 	 then 1
			 else x * fact(x - 1).

print fact(0).
print fact(7).-]

%% Fibonacci
[-
let fib = lambda (n) {
	let iter = lambda (a, b, n)
			   	 if n == 0
				 then a
				 else iter(b, a + b, n - 1).
	iter(0, 1, n)
}.

print fib(0).
print fib(1).
print fib(2).
print fib(3).
-]