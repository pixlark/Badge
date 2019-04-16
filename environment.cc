struct Environment {
	GC_List<Symbol> names;
	GC_List<size_t> offsets;
	static Environment * alloc()
	{
		Environment * env = (Environment*) GC::alloc(sizeof(Environment));
		env->names.alloc();
		env->offsets.alloc();
		return env;
	}
	void gc_mark()
	{
		names.gc_mark();
		offsets.gc_mark();
	}
	bool is_bound(Symbol symbol)
	{
		assert(names.size == offsets.size);
		for (int i = 0; i < names.size; i++) {
			if (symbol == names[i]) {
				return true;
			}
		}
		return false;
	}
	bool create_binding(Symbol symbol, size_t offset)
	{
		assert(names.size == offsets.size);
		if (is_bound(symbol)) {
			return false;
		}
		names.push(symbol);
		offsets.push(offset);
		return true;
	}
	bool update_binding(Symbol symbol, size_t offset)
	{
		assert(names.size == offsets.size);
		for (int i = 0; i < names.size; i++) {
			if (symbol == names[i]) {
				offsets[i] = offset;
				return true;
			}
		}
		return false;
	}
	bool resolve_binding(Symbol symbol, size_t * offset)
	{
		assert(names.size == offsets.size);
		for (int i = 0; i < names.size; i++) {
			if (symbol == names[i]) {
				*offset = offsets[i];
				return true;
			}
		}
		return false;
	}
	static void test()
	{
		auto env = Environment::alloc();
		assert( env->create_binding(Intern::intern("asdf"), 5));
		assert( env->create_binding(Intern::intern("j"), 0));
		assert(!env->create_binding(Intern::intern("asdf"), 6));
		assert( env->update_binding(Intern::intern("asdf"), 100));
		size_t resolved;
		assert(env->resolve_binding(Intern::intern("asdf"), &resolved));
		assert(resolved == 100);
	}
};
