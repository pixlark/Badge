def isprime(n):
	def helper(n, i):
		if i == n:
			return True
		elif n % i == 0:
			return False
		else:
			return helper(n, i + 1)
	return helper(n, 2)

def loop(n, m):
	if n >= 50:
		return
	print(n, end='')
	print(" ", end='')
	if isprime(n):
		print("prime!", end='')
	else:
		print("not prime", end='')
	print('')
	loop(n + 1, m)

loop(2, 50)
