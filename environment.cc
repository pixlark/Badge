struct Environment {
	GC_List<Symbol> names;
	GC_List<Value> values;

	Environment * next_env;
	
	static Environment * alloc()
	{
		Environment * env = (Environment*) GC::alloc(sizeof(Environment));
		env->names.alloc();
		env->values.alloc();
		env->next_env = NULL;
		return env;
	}
	void gc_mark()
	{
		names.gc_mark();
		values.gc_mark();
		for (int i = 0; i < values.size; i++) {
			values[i].gc_mark();
		}
		if (next_env) {
			GC::mark_opaque(next_env);
			next_env->gc_mark();
		}
	}
	bool is_bound(Symbol symbol)
	{
		assert(names.size == values.size);
		for (int i = 0; i < names.size; i++) {
			if (symbol == names[i]) {
				return true;
			}
		}
		if (next_env) {
			return next_env->is_bound(symbol);
		}
		return false;
	}
	bool create_binding(Symbol symbol, Value value)
	{
		assert(names.size == values.size);
		// TODO(pixlark): Do we want to be able to shadow closed
		// variables? Probably...
		if (is_bound(symbol)) {
			return false;
		}
		names.push(symbol);
		values.push(value);
		return true;
	}
	bool update_binding(Symbol symbol, Value value)
	{
		assert(names.size == values.size);
		for (int i = 0; i < names.size; i++) {
			if (symbol == names[i]) {
				values[i] = value;
				return true;
			}
		}
		if (next_env) {
			return next_env->update_binding(symbol, value);
		}
		return false;
	}
	bool resolve_binding(Symbol symbol, Value * value)
	{
		assert(names.size == values.size);
		for (int i = 0; i < names.size; i++) {
			if (symbol == names[i]) {
				*value = values[i];
				return true;
			}
		}
		if (next_env) {
			return next_env->resolve_binding(symbol, value);
		}
		return false;
	}
};
