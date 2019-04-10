struct Function {
	Symbol * parameters;
	size_t parameter_count;
	List<BC> * bytecode;
};

char * Value::to_string()
{
	switch (type) {
	case TYPE_INTEGER:
		return itoa(integer);
	case TYPE_SYMBOL:
		return strdup(symbol);
	case TYPE_FUNCTION:
		return strdup("@[function]");
	}
}
