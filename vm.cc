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
	Blocks * blocks;
	
	List<Value> stack;
	List<Scope*> scopes;

	BC * bytecode;
	size_t bc_pointer;
	size_t bc_length;
	
	void init(Blocks * blocks)
	{
		this->blocks = blocks;
		stack.alloc();
		scopes.alloc();
		scopes.push(Scope::create_empty());
	}
	void prime(size_t block_reference)
	{
		bytecode = blocks->retrieve_block(block_reference);
		bc_pointer = 0;
		bc_length = blocks->size_block(block_reference);
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
		return bc_pointer >= bc_length;
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
		for (int i = 0; i < stack.size; i++) {
			stack[i].gc_mark();
		}
	}
	void step()
	{	
		if (halted()) {
			return;
		}
		BC bc = bytecode[bc_pointer++];
		switch (bc.kind) {
		case BC_LOAD_CONST: {
			push(bc.arg.value);
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
			auto count = pop();
			count.assert_is(TYPE_INTEGER);
			
			Function * func =  (Function*) GC::alloc(sizeof(Function));
			func->block_reference = bc.arg.block_reference;
			
			func->parameter_count = count.integer;
			func->parameters = (Symbol*) GC::alloc(sizeof(Symbol) * count.integer);
			for (int i = 0; i < count.integer; i++) {
				auto it = pop();
				it.assert_is(TYPE_SYMBOL);
				func->parameters[count.integer - i - 1] = it.symbol;
			}

			Value value = Value::create(TYPE_FUNCTION);
			value.ref_function = func;
			push(value);
		} break;
		}

		GC::unmark_all();
		mark_reachable();
		GC::free_unmarked();
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
