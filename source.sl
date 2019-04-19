let GCD = lambda (a, b)
	if a == b
	then a
	elif a > b
	then GCD(a - b, b)
	else GCD(a, b - a).

print GCD(12, 16).
