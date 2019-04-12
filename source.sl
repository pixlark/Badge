

let f = lambda () { print f; };
f();

set f = lambda (x) { print x; };
f(f);
f(2);

set f = lambda (a) {
	return lambda (b) { return b; };
};

let add1 = f(1);
print add1(2);


