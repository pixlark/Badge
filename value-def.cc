struct Function {
	Symbol * parameters;
	size_t parameter_count;
	size_t block_reference;
};

char * Value::to_string()
{
	switch (type) {
	case TYPE_NOTHING:
		return strdup("nothing");
	case TYPE_INTEGER:
		return itoa(integer);
	case TYPE_SYMBOL:
		return strdup(symbol);
	case TYPE_FUNCTION:
		return strdup("@[function]");
	}
	assert(false); // @linter
}

void Value::gc_mark()
{
	switch (type) {
	case TYPE_NOTHING:
		break;
	case TYPE_INTEGER:
		break;
	case TYPE_SYMBOL:
		break;
	case TYPE_FUNCTION:
		GC::mark_opaque(ref_function);
		GC::mark_opaque(ref_function->parameters);
		break;
	}
}

static void validate_same_type(Value a, Value b)
{
	if (!a.same_type(b)) {
		fatal("Values must be of same type");
	}
}

Value Value::add(Value a, Value b)
{
	validate_same_type(a, b);
	switch (a.type) {
	case TYPE_INTEGER:
		return Value::raise(a.integer + b.integer);
	default:
		fatal("+ not valid for type");
	}
	return (Value) {}; // @linter
}

Value Value::subtract(Value a, Value b)
{
	validate_same_type(a, b);
	switch (a.type) {
	case TYPE_INTEGER:
		return Value::raise(a.integer - b.integer);
	default:
		fatal("- not valid for type");
	}
	return (Value) {}; // @linter
}

Value Value::multiply(Value a, Value b)
{
	validate_same_type(a, b);
	switch (a.type) {
	case TYPE_INTEGER:
		return Value::raise(a.integer * b.integer);
	default:
		fatal("* not valid for type");
	}
	return (Value) {}; // @linter
}

Value Value::divide(Value a, Value b)
{
	validate_same_type(a, b);
	switch (a.type) {
	case TYPE_INTEGER:
		return Value::raise(a.integer / b.integer);
	default:
		fatal("/ not valid for type");
	}
	return (Value) {}; // @linter
}
