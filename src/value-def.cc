struct Function {
	Symbol * parameters;
	size_t parameter_count;
	size_t block_reference;
	Environment * closure;
};

struct Builtin {
	Symbol name;
	size_t arg_count;
	Value(*funcptr)(Value *);
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
	case TYPE_BUILTIN:
		return strdup("@[builtin]");
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
		if (!GC::is_marked_opaque(ref_function->closure)) {
			GC::mark_opaque(ref_function->closure);
			ref_function->closure->gc_mark();
		}
		break;
	case TYPE_BUILTIN:
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
	assert(false); // @linter
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
	assert(false); // @linter
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
	assert(false); // @linter
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
	assert(false); // @linter
}

bool Value::equal(Value a, Value b)
{
	if (a.type != b.type) {
		return false;
	}
	switch (a.type) {
	case TYPE_NOTHING:
		return true;
	case TYPE_INTEGER:
		return a.integer == b.integer;
	case TYPE_SYMBOL:
		return a.symbol == b.symbol;
	case TYPE_FUNCTION:
		return a.ref_function == b.ref_function;
	case TYPE_BUILTIN:
		return a.builtin == b.builtin;
	}
	assert(false); // @linter
}

bool Value::_and(Value a, Value b)
{
	return a.truthy() && b.truthy();
}

bool Value::_or(Value a, Value b)
{
	return a.truthy() || b.truthy();
}

bool Value::less_than(Value a, Value b)
{
	validate_same_type(a, b);
	switch (a.type) {
	case TYPE_INTEGER:
		return a.integer < b.integer;
	default:
		fatal("< not valid for type");
	}
	assert(false); // @linter
}

bool Value::greater_than(Value a, Value b)
{
	validate_same_type(a, b);
	switch (a.type) {
	case TYPE_INTEGER:
		return a.integer > b.integer;
	default:
		fatal("> not valid for type");
	}
	assert(false); // @linter
}

bool Value::less_than_or_equal_to(Value a, Value b)
{
	validate_same_type(a, b);
	switch (a.type) {
	case TYPE_INTEGER:
		return a.integer <= b.integer;
	default:
		fatal("<= not valid for type");
	}
	assert(false); // @linter
}

bool Value::greater_than_or_equal_to(Value a, Value b)
{
	validate_same_type(a, b);
	switch (a.type) {
	case TYPE_INTEGER:
		return a.integer >= b.integer;
	default:
		fatal(">= not valid for type");
	}
	assert(false); // @linter
}
