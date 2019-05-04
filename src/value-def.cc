struct String {
	char * string;
	size_t length;
	void gc_mark()
	{
		GC::mark_opaque(string);
	}
};

struct Function {
	Symbol * parameters;
	size_t parameter_count;
	size_t block_reference;
	Environment * closure;
	void gc_mark()
	{
		GC::mark_opaque(parameters);
		if (!GC::is_marked_opaque(closure)) {
			GC::mark_opaque(closure);
			closure->gc_mark();
		}
	}
};

struct Builtin {
	Symbol name;
	size_t arg_count;
	Value(*funcptr)(Value *);
};

struct Constructor {
	Symbol * fields;
	size_t field_count;
	void gc_mark()
	{
		GC::mark_opaque(fields);
	}
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
	case TYPE_STRING: {
		char * s = (char*) malloc(sizeof(char) * (ref_string->length + 1));
		strncpy(s, ref_string->string, ref_string->length);
		s[ref_string->length] = '\0';
		return s;
	};
	case TYPE_FUNCTION:
		return strdup("@[function]");
	case TYPE_BUILTIN:
		return strdup("@[builtin]");
	case TYPE_CONSTRUCTOR:
		return strdup("@[constructor]");
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
	case TYPE_STRING:
		GC::mark_opaque(ref_string);
		ref_string->gc_mark();
		break;
	case TYPE_FUNCTION:
		GC::mark_opaque(ref_function);
		ref_function->gc_mark();
		break;
	case TYPE_BUILTIN:
		break;
	case TYPE_CONSTRUCTOR:
		GC::mark_opaque(ref_constructor);
		ref_constructor->gc_mark();
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
	case TYPE_STRING:
		if (a.ref_string->length != b.ref_string->length) {
			return false;
		}
		return strncmp(a.ref_string->string,
					   b.ref_string->string,
					   a.ref_string->length);
	case TYPE_FUNCTION:
		return a.ref_function == b.ref_function;
	case TYPE_BUILTIN:
		return a.builtin == b.builtin;
	case TYPE_CONSTRUCTOR:
		return a.ref_constructor == b.ref_constructor;
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
