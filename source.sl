let fib = lambda (n) {
	let iter =
		lambda (a, b, n) if n == 0
				   	  	 then a
				 		 else iter(b, a + b, n - 1).
	iter(0, 1, n)
}.

let loop = lambda (m, n) if n == m
	   	   		  	  	 then nothing
					  	 else {
		   			  	     print fib(n).
						     loop(m, n + 1)
		 			     }.

loop(10, 0).
