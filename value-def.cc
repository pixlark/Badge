struct Value_Bytecode {
	GC_List<BC> bytecode;
};

struct Value_Function {
	GC_List<Symbol> parameters;
	GC_List<BC> bytecode;
};

char * Value::to_string()
{
	String_Builder builder;
	switch (type) {
	case TYPE_INTEGER: {
		char * s = itoa(integer);
		builder.append(s);
		free(s);
	} break;
	case TYPE_SYMBOL:
		builder.append(symbol);
		break;
	case TYPE_BYTECODE:
		builder.append("[[bytecode]]");
		break;
	case TYPE_FUNCTION: {
		builder.append("[[function; ");
		char * s = itoa(ref_function->parameters.size);
		builder.append(s);
		free(s);
		builder.append("-ary]]");
	} break;
	}
	return builder.final_string();
}
