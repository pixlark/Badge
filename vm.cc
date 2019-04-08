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

	List<BC> bytecode;
	size_t bc_pointer;
	
	void init()
	{
		stack.alloc();
		scopes.alloc();
		scopes.push(Scope::create_empty());
	}
	void prime(List<BC> bytecode)
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
	void step()
	{
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
			push(stack[offset]);
		} break;
		case BC_POP_AND_PRINT: {
			auto v = pop();
			char * s = v.to_string();
			printf("%s\n", s);
			free(s);
		} break;
		}
	}
	void print_debug_info()
	{
		printf("--- Frame ---\n");
		const int width = 13;
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
		printf("-------------\n\n");
	}
};
