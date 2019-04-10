enum BC_Kind {
	BC_LOAD_CONST,
	BC_CREATE_BINDING,
	BC_UPDATE_BINDING,
	BC_RESOLVE_BINDING,
	BC_POP_AND_PRINT,
	BC_CONSTRUCT_FUNCTION,
};

static const char * BC_Kind_names[] = {
	"LOAD_CONST",
	"CREATE_BINDING",
	"UPDATE_BINDING",
	"RESOLVE_BINDING",
	"POP_AND_PRINT",
	"CONSTRUCT_FUNCTION",
};

struct BC {
	BC_Kind kind;
	union {
		Value value;
		int integer;
		List<BC> bytecode;
	} arg;
	static BC create(BC_Kind kind)
	{
		BC bc;
		bc.kind = kind;
		return bc;
	}
	static BC create(BC_Kind kind, Value arg)
	{
		BC bc;
		bc.kind = kind;
		bc.arg.value = arg;
		return bc;
	}
	static BC create(BC_Kind kind, int arg)
	{
		BC bc;
		bc.kind = kind;
		bc.arg.integer = arg;
		return bc;
	}
	static BC create(BC_Kind kind, List<BC> arg)
	{
		BC bc;
		bc.kind = kind;
		bc.arg.bytecode = arg;
		return bc;
	}
	char * to_string()
	{
		String_Builder builder;
		builder.append(BC_Kind_names[kind]);
		// TODO(pixlark): Fancier
		return builder.final_string();
	}
};
