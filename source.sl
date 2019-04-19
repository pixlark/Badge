








let f = lambda () {
	{
		let x = 2.
	}.
	print x.
}.

f().






[-
{
	let fact = lambda (x) if x == 0
			   		  	  then 1
						  else x * fact(x - 1).
	print fact(7).
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