enum Type {
	TYPE_NOTHING,
	TYPE_INTEGER,
	TYPE_SYMBOL,
	TYPE_FUNCTION,
	TYPE_FFI,
};

struct Function;
struct FFI;

struct Value {
	Type type;
	union {
		int integer;
		Symbol symbol;
		Function * ref_function;
		FFI * ffi;
	};
	static Value create(Type type)
	{
		return (Value) { type };
	}
	static Value nothing()
	{
		return (Value) { TYPE_NOTHING };
	}
	static Value raise(int integer)
	{
		Value v = { TYPE_INTEGER };
		v.integer = integer;
		return v;
	}
	static Value raise(Symbol symbol)
	{
		Value v = { TYPE_SYMBOL };
		v.symbol = symbol;
		return v;
	}
	static Value raise_bool(bool b) // Can't be an overload because
									// C++ is stupid
	{
		if (b) {
			return Value::raise(1);
		} else {
			return Value::nothing();
		}
	}
	bool truthy()
	{
		return this->type != TYPE_NOTHING;
	}
	bool is(Type type)
	{
		return this->type == type;
	}
	void assert_is(Type type)
	{
		assert(is(type));
	}
	bool same_type(Value other)
	{
		return type == other.type;
	}
	char * to_string();
	void gc_mark();

	static Value add(Value a, Value b);
	static Value subtract(Value a, Value b);
	static Value multiply(Value a, Value b);
	static Value divide(Value a, Value b);
	static bool equal(Value a, Value b);
	static bool less_than(Value a, Value b);
	static bool greater_than(Value a, Value b);
	static bool less_than_or_equal_to(Value a, Value b);
	static bool greater_than_or_equal_to(Value a, Value b);
	static bool _and(Value a, Value b);
	static bool _or(Value a, Value b);
};
