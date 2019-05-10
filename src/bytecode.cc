enum BC_Kind {
	// basic
	BC_NOP = 0,
	BC_POP_AND_DISCARD,
	BC_LOAD_CONST,
	BC_DUPLICATE,
	// binding
	BC_CREATE_BINDING,
	BC_UPDATE_BINDING,
	BC_RESOLVE_BINDING,
	// arithmetic
	BC_ADD,
	BC_SUBTRACT,
	BC_MULTIPLY,
	BC_DIVIDE,
	BC_NEGATE,
	// comparison
	BC_EQUAL,
	BC_NOT_EQUAL,
	BC_GREATER_THAN,
	BC_LESS_THAN,
	BC_GREATER_THAN_OR_EQUAL_TO,
	BC_LESS_THAN_OR_EQUAL_TO,
	// functions
	BC_CONSTRUCT_FUNCTION,
	BC_POP_AND_CALL_FUNCTION,
	BC_RETURN,
	BC_THIS_FUNCTION,
	// strings
	BC_SYMBOL_TO_STRING,
	// boolean
	BC_AND,
	BC_OR,
	BC_NOT,
	// flow control
	BC_JUMP,
	BC_POP_JUMP,
	// scoping
	BC_ENTER_SCOPE,
	BC_EXIT_SCOPE,
	// structs
	BC_CONSTRUCT_CONSTRUCTOR,
	BC_RESOLVE_FIELD,
	BC_UPDATE_FIELD,
	// bodies
	BC_PUSH_BODY,
	BC_BREAK_BODY,
	// file units
	BC_RUN_FILE_UNIT,
	BC_EXPORT_SYMBOL,
};

static const char * BC_Kind_names[] = {
	"NOP",
	"POP_AND_DISCARD",
	"LOAD_CONST",
	"DUPLICATE",
	"CREATE_BINDING",
	"UPDATE_BINDING",
	"RESOLVE_BINDING",
	"ADD",
	"SUBTRACT",
	"MULTIPLY",
	"DIVIDE",
	"NEGATE",
	"EQUAL",
	"NOT_EQUAL",
	"GREATER_THAN",
	"LESS_THAN",
	"GREATER_THAN_OR_EQUAL_TO",
	"LESS_THAN_OR_EQUAL_TO",
	"CONSTRUCT_FUNCTION",
	"POP_AND_CALL_FUNCTION",
	"RETURN",
	"THIS_FUNCTION",
	"SYMBOL_TO_STRING",
	"AND",
	"OR",
	"NOT",
	"JUMP",
	"POP_JUMP",
	"ENTER_SCOPE",
	"EXIT_SCOPE",
	"CONSTRUCT_CONSTRUCTOR",
	"RESOLVE_FIELD",
	"UPDATE_FIELD",
	"PUSH_BODY",
	"BREAK_BODY",
	"RUN_FILE_UNIT",
	"EXPORT_SYMBOL",
};

struct BC {
	BC_Kind kind;
	Assoc_Ptr assoc;
	union {
		Value value;
		int integer;
		size_t block_reference;
	} arg;
	static BC create(BC_Kind kind, Assoc_Ptr assoc)
	{
		BC bc;
		bc.kind = kind;
		bc.assoc = assoc;
		return bc;
	}
	static BC create(BC_Kind kind, Value arg, Assoc_Ptr assoc)
	{
		BC bc;
		bc.kind = kind;
		bc.arg.value = arg;
		bc.assoc = assoc;
		return bc;
	}
	static BC create(BC_Kind kind, int arg, Assoc_Ptr assoc)
	{
		BC bc;
		bc.kind = kind;
		bc.arg.integer = arg;
		bc.assoc = assoc;
		return bc;
	}
	static BC create(BC_Kind kind, size_t arg, Assoc_Ptr assoc)
	{
		BC bc;
		bc.kind = kind;
		bc.arg.block_reference = arg;
		bc.assoc = assoc;
		return bc;
	}
	char * to_string()
	{
		String_Builder builder;
		builder.append(BC_Kind_names[kind]);
		builder.append(" ");
		switch (kind) {
		case BC_LOAD_CONST: {
			char * s = arg.value.to_string();
			defer { free(s); };
			builder.append(s);
		} break;
		case BC_JUMP:
		case BC_POP_JUMP: {
			char * s = itoa(arg.integer);
			defer { free(s); };
			builder.append(s);
		} break;
		default:
			break;
		}
		/*
		builder.append(" : ");
		{
			char buf[512];
			sprintf(buf, "%p %zu", assoc.source, assoc.position);
			builder.append(buf);
			}*/
		return builder.final_string();
	}
};
