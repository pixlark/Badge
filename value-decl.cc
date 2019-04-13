enum Type {
	TYPE_NOTHING,
	TYPE_INTEGER,
	TYPE_SYMBOL,
	TYPE_FUNCTION,
};

struct Function;

struct Value {
	Type type;
	union {
		int integer;
		Symbol symbol;
		Function * ref_function;
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
	void assert_is(Type type)
	{
		assert(this->type == type);
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
};
