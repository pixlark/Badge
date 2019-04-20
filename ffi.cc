#include <math.h>

/* FOREIGN FUNCTION INTERFACE
 *
 * Our foreign function interface is very spartan and not at all
 * easy-to-use. If you need to add a new FFI function, follow these
 * steps:
 *
 * 1. Determine the header file it came from, and write your wrapper
 *    function in that category
 * 2. Give the function an enum name in the Foreign_Function enum
 * 3. Add a CASE to lower_from_symbol that takes in your function name
 *    and returns your function enum.
 * 4. Add the argument count to the ffi_argument_counts array
 * 5. Add the function pointer to the ffi_funcptrs array
 * 
 */

#define DEFINE(name) Value v_##name(Value * args)
#define NAMEOF(name) (v_##name)
#define PULL_ONE(a) Value a = args[0]
#define PULL_TWO(a, b) Value a = args[0], b = args[1]
#define PULL_THREE(a, b, c) Value a = args[0], b = args[1], c = args[2]
#define PULL_FOUR(a, b, c, d) Value a = args[0], b = args[1], c = args[2], d = args[3]

namespace Foreign {
	Map<Symbol, FFI*> foreign_functions;
	// Math.h wrappers
	DEFINE(abs)
	{
		PULL_ONE(n);
		if (n.type != TYPE_INTEGER) {
			fatal("FFI function abs() takes integer");
		}
		return Value::raise(abs(n.integer));
	}
	// FFI bridge
	enum Foreign_Function {
		FFI_MATH_ABS,
	};
	size_t ffi_argument_counts[] = {
		[FFI_MATH_ABS] = 1,
	};
	Value(*ffi_funcptrs[])(Value *) = {
		[FFI_MATH_ABS] = NAMEOF(abs),
	};
	// FFI interface
	bool symbol_comparator(Symbol a, Symbol b) { return a == b; }
	void init()
	{
		foreign_functions.alloc(symbol_comparator);
	}
	void destroy()
	{
		for (int i = 0; i < foreign_functions.values.size; i++) {
			free(foreign_functions.values[i]);
		}
		foreign_functions.dealloc();
	}
	#define CASE(a, b) else if (symbol == Intern::intern(a)) return (b)
	Foreign_Function lower_from_symbol(Symbol symbol)
	{
		if (0) assert(false);
		CASE("abs", FFI_MATH_ABS);
		else fatal("Foreign function '%s' does not exist!", symbol);
		assert(false); // @linter
	}
	#undef CASE
	FFI * get_ffi(Symbol symbol)
	{
		if (foreign_functions.bound(symbol)) {
			return foreign_functions.lookup(symbol);
		}
		// FFI not yet loaded, allocate and load
		FFI * ffi = (FFI*) malloc(sizeof(FFI));
		Foreign_Function kind = lower_from_symbol(symbol);
		ffi->name = symbol;
		ffi->arg_count = ffi_argument_counts[kind];
		ffi->funcptr = ffi_funcptrs[kind];
		foreign_functions.add(symbol, ffi);
		return ffi;
	}
}

#undef PULL_FOUR
#undef PULL_THREE
#undef PULL_TWO
#undef PULL_ONE
#undef NAMEOF
#undef DEFINE
