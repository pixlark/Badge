struct Closure {
	size_t size;
	Symbol * names;
	Value * values;
	static Closure create(size_t size)
	{
		Closure c;
		c.size = size;
		c.names = (Symbol*) GC::alloc(sizeof(Symbol) * size);
		c.values = (Value*) GC::alloc(sizeof(Value) * size);
		return c;
	}
	void gc_mark()
	{
		GC::mark_opaque(names);
		GC::mark_opaque(values);
		for (int i = 0; i < size; i++) {
			values[i].gc_mark();
		}
	}
};

struct Function {
	Symbol * parameters;
	size_t parameter_count;
	size_t block_reference;
	Closure closure;
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
		ref_function->closure.gc_mark();
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
	}
	assert(false); // @linter
}

Value Value::_and(Value a, Value b)
{
	if (a.type == TYPE_NOTHING ||
		b.type == TYPE_NOTHING) {
		return Value::nothing();
	} else {
		return Value::raise(1);
	}
}

Value Value::_or(Value a, Value b)
{
	if (a.type == TYPE_NOTHING &&
		b.type == TYPE_NOTHING) {
		return Value::nothing();
	} else {
		return Value::raise(1);
	}
}
