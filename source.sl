
%% This shouldn't work
%lambda (x) { print x; }(1);
%% This should work
(lambda (x) { print x; })(1);

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
