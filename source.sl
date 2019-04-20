let mod = @ffi[mod].

let isprime = lambda (n) {
	let helper = lambda (n, i) if   i == n
			   	 			   then 1
							   elif mod(n, i) == 0
		   	 			   	   then nothing
						   	   else helper(n, i + 1).
	helper(n, 2)
}.

let loop = lambda (n, m) {
	if n >= 50
	then nothing
	else {
		print n.
		print isprime(n).
		loop(n + 1, m).
	}.
}.

loop(2, 50).
