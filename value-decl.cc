enum Type {
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
	char * to_string();
	void gc_mark();
};
