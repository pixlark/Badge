#include <math.h>

/* BUILTIN FUNCTIONS
 *
 * If you need to add a new builtin function, follow these steps:
 *
 * 1. Give the function an enum name in the Foreign_Function enum
 * 2. Add a CASE to lower_from_symbol that takes in your function name
 *    and returns your function enum.
 * 3. Add the argument count to the builtin_argument_counts array
 * 4. Add the function pointer to the builtin_funcptrs array
 * 
 */

#define DEFINE(name) Value v_##name(Value * args)
#define NAMEOF(name) (v_##name)
#define PULL_ONE(a) Value a = args[0]
#define PULL_TWO(a, b) Value a = args[0], b = args[1]
#define PULL_THREE(a, b, c) Value a = args[0], b = args[1], c = args[2]
#define PULL_FOUR(a, b, c, d) Value a = args[0], b = args[1], c = args[2], d = args[3]

namespace Builtins {
	Map<Symbol, Builtin*> builtin_functions;
	// Math functions
	DEFINE(abs)
	{
		PULL_ONE(n);
		if (n.type != TYPE_INTEGER) {
			fatal("Builtin function abs() takes integer");
		}
		return Value::raise(abs(n.integer));
	}
	DEFINE(mod)
	{
		PULL_TWO(n, b);
		if (!n.is(TYPE_INTEGER) || !b.is(TYPE_INTEGER)) {
			fatal("FFI function mod() takes integers");
		}
		return Value::raise(n.integer % b.integer);
	}
	// IO functions
	DEFINE(print)
	{
		PULL_ONE(v);
		char * s = v.to_string();
		printf("%s", s);
		free(s);
		return Value::nothing();
	}
	DEFINE(println)
	{
		PULL_ONE(v);
		char * s = v.to_string();
		printf("%s\n", s);
		free(s);
		return Value::nothing();
	}	
	// Bridge
	enum Builtin_Function {
		BUILTIN_MATH_ABS,
		BUILTIN_MATH_MOD,
		BUILTIN_IO_PRINT,
		BUILTIN_IO_PRINTLN,
	};
	size_t builtin_argument_counts[] = {
		[BUILTIN_MATH_ABS] = 1,
		[BUILTIN_MATH_MOD] = 2,
		[BUILTIN_IO_PRINT] = 1,
		[BUILTIN_IO_PRINTLN] = 1,		
	};
	Value(*builtin_funcptrs[])(Value *) = {
		[BUILTIN_MATH_ABS] = NAMEOF(abs),
		[BUILTIN_MATH_MOD] = NAMEOF(mod),
		[BUILTIN_IO_PRINT] = NAMEOF(print),
		[BUILTIN_IO_PRINTLN] = NAMEOF(println),
	};
	// FFI interface
	bool symbol_comparator(Symbol a, Symbol b) { return a == b; }
	void init()
	{
		builtin_functions.alloc(symbol_comparator);
	}
	void destroy()
	{
		for (int i = 0; i < builtin_functions.values.size; i++) {
			free(builtin_functions.values[i]);
		}
		builtin_functions.dealloc();
	}
	#define CASE(a, b) else if (symbol == Intern::intern(a)) return (b)
	Builtin_Function lower_from_symbol(Symbol symbol)
	{
		if (0) assert(false);
		CASE("abs", BUILTIN_MATH_ABS);
		CASE("mod", BUILTIN_MATH_MOD);
		CASE("print", BUILTIN_IO_PRINT);
		CASE("println", BUILTIN_IO_PRINTLN);
		else fatal("Foreign function '%s' does not exist!", symbol);
		assert(false); // @linter
	}
	#undef CASE
	Builtin * get_builtin(Symbol symbol)
	{
		if (builtin_functions.bound(symbol)) {
			return builtin_functions.lookup(symbol);
		}
		// FFI not yet loaded, allocate and load
		Builtin * ffi = (Builtin*) malloc(sizeof(Builtin));
		Builtin_Function kind = lower_from_symbol(symbol);
		ffi->name = symbol;
		ffi->arg_count = builtin_argument_counts[kind];
		ffi->funcptr = builtin_funcptrs[kind];
		builtin_functions.add(symbol, ffi);
		return ffi;
	}
}

#undef PULL_FOUR
#undef PULL_THREE
#undef PULL_TWO
#undef PULL_ONE
#undef NAMEOF
#undef DEFINE
