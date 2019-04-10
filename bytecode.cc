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
		Value arg;
	};
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
		bc.arg = arg;
		return bc;
	}
	bool has_arg()
	{
		return kind == BC_LOAD_CONST;
	}
	char * to_string()
	{
		String_Builder builder;
		builder.append(BC_Kind_names[kind]);
		builder.append(" ");
		if (has_arg()) {
			char * s = arg.to_string();
			builder.append(s);
			free(s);
		}
		return builder.final_string();
	}
};
