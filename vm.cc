struct Scope {
	List<Symbol> symbols;
	List<size_t> offsets;
	void init()
	{
		symbols.alloc();
		offsets.alloc();
	}
	void destroy()
	{
		symbols.dealloc();
		offsets.dealloc();
	}
	static Scope * create_empty()
	{
		Scope * s = (Scope*) malloc(sizeof(Scope));
		s->init();
		return s;
	}
	int where_bound(Symbol symbol)
	{
		for (int i = 0; i < symbols.size; i++) {
			if (symbol == symbols[i]) {
				return i;
			}
		}
		return -1;
	}
	bool create_binding(Symbol symbol, size_t offset)
	{
		if (where_bound(symbol) != -1) {
			return false;
		}
		assert(symbols.size == offsets.size);
		symbols.push(symbol);
		offsets.push(offset);
		return true;
	}
	bool resolve_binding(Symbol symbol, size_t * ret)
	{
		assert(symbols.size == offsets.size);
		for (int i = 0; i < symbols.size; i++) {
			if (symbol == symbols[i]) {
				*ret = offsets[i];
				return true;
			}
		}
		return false;
	}
	static void test()
	{
		Scope scope;
		scope.init();

		scope.create_binding(Intern::intern("test"), 13);
		size_t ret;
		scope.resolve_binding(Intern::intern("test"), &ret);
		printf("%d\n", ret);
		
		scope.destroy();
	}
};

struct VM {
	List<Value> stack;
	List<Scope*> scopes;

	GC_List<BC> bytecode;
	size_t bc_pointer;
	
	void init()
	{
		stack.alloc();
		scopes.alloc();
		scopes.push(Scope::create_empty());
	}
	void prime(GC_List<BC> bytecode)
	{
		this->bytecode = bytecode;
		bc_pointer = 0;
	}
	void destroy()
	{
		stack.dealloc();
		scopes[0]->destroy();
		free(scopes[0]);
		scopes.dealloc();
	}
	bool halted()
	{
		return bc_pointer >= bytecode.size;
	}
	void push(Value v)
	{
		stack.push(v);
	}
	Value pop()
	{
		return stack.pop();
	}
	size_t top_offset()
	{
		return stack.size - 1;
	}
	Scope * current_scope()
	{
		return scopes[scopes.size - 1];
	}
	void mark_reachable()
	{
		
	}
	void step()
	{
		// Garbage collection
		GC::unmark_all();
		mark_reachable();
		GC::free_unmarked();
		
		if (halted()) {
			return;
		}
		BC bc = bytecode[bc_pointer++];
		switch (bc.kind) {
		case BC_LOAD_CONST: {
			push(bc.arg);
		} break;
		case BC_CREATE_BINDING: {
			auto symbol = pop();
			symbol.assert_is(TYPE_SYMBOL);
			bool success = current_scope()->create_binding(symbol.symbol, top_offset());
			if (!success) {
				fatal("Can't create new variable '%s' -- already bound!", symbol.symbol);
			}
		} break;
		case BC_UPDATE_BINDING: {
			auto symbol = pop();
			symbol.assert_is(TYPE_SYMBOL);
			auto new_value = pop();
			size_t offset;
			bool success = current_scope()->resolve_binding(symbol.symbol, &offset);
			if (!success) {
				fatal("Can't update variable '%s' -- not bound!", symbol.symbol);
			}
			stack[offset] = new_value;
		} break;
		case BC_RESOLVE_BINDING: {
			auto symbol = pop();
			symbol.assert_is(TYPE_SYMBOL);
			size_t offset;
			bool success = current_scope()->resolve_binding(symbol.symbol, &offset);
			if (!success) {
				fatal("Variable '%s' is not bound", symbol.symbol);
			}
			assert(offset < stack.size);
			push(stack[offset]);
		} break;
		case BC_POP_AND_PRINT: {
			auto v = pop();
			char * s = v.to_string();
			printf("%s\n", s);
			free(s);
		} break;
		case BC_CONSTRUCT_FUNCTION: {
			auto value_bytecode = pop();
			value_bytecode.assert_is(TYPE_BYTECODE);
			auto param_count = pop();
			param_count.assert_is(TYPE_INTEGER);
			List<Value> symbols;
			symbols.alloc();
			for (int i = 0; i < param_count.integer; i++) {
				symbols.push(
			}
		} break;
		}
	}
	void print_debug_info()
	{
		printf("--- Frame ---\n");
		const int width = 13;
		if (bc_pointer > 0) {
			char * s = bytecode[bc_pointer - 1].to_string();
			printf("%s\n", s);
			free(s);
		}
		printf(".............\n");
		printf("    Stack\n");
		for (int i = top_offset(); i >= 0; i--) {
			char * s = stack[i].to_string();
			size_t len = strlen(s);

			if (len < width) {
				int spacing = (width - len) / 2;
				for (int i = 0; i < spacing; i++) {
					printf(" ");
				}
			}

			printf("%s\n", s);
			free(s);
		}
		printf(".............\n");
		printf("    Vars\n");
		for (int i = scopes.size - 1; i >= 0; i--) {
			auto scope = scopes[i];
			assert(scope->symbols.size == scope->offsets.size);
			for (int j = 0; j < scope->symbols.size; j++) {
				printf("  %s: %d\n", scope->symbols[j], scope->offsets[j]);
			}
			if (i < scopes.size - 1) {
				printf("      .\n");
			}
		}
		printf("-------------\n\n");
	}
};
