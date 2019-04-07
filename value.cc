enum Type {
	TYPE_INTEGER,
	TYPE_SYMBOL,
};

struct Value {
	Type type;
	union {
		int integer;
		Symbol symbol;
	};
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
	char * to_string()
	{
		switch (type) {
		case TYPE_INTEGER:
			return itoa(integer);
		case TYPE_SYMBOL:
			return strdup(symbol);
		default:
			fatal_internal("Incomplete switch: Value.to_string()");
		}
	}
};
