enum Type {
	TYPE_INTEGER,
	TYPE_SYMBOL,
	TYPE_BYTECODE,
	TYPE_FUNCTION,
};

struct Value_Bytecode;
struct Value_Function;

struct Value {
	Type type;
	union {
		int integer;
		Symbol symbol;
		Value_Bytecode * ref_bytecode;
		Value_Function * ref_function;
	};
	static Value raise(int x)
	{
		Value v = { TYPE_INTEGER };
		v.integer = x;
		return v;
	}
	static Value raise(Symbol x)
	{
		Value v = { TYPE_SYMBOL };
		v.symbol = x;
		return v;
	}
	static Value raise(Value_Bytecode * x)
	{
		Value v = { TYPE_BYTECODE };
		v.ref_bytecode = x;
		return v;
	}
	static Value raise(Value_Function * x)
	{
		Value v = { TYPE_FUNCTION };
		v.ref_function = x;
		return v;
	}
	void assert_is(Type type)
	{
		assert(this->type == type);
	}
	char * to_string(); 
};

