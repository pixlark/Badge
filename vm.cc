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
	static Scope * alloc_empty()
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
		printf("%zu\n", ret);
		
		scope.destroy();
	}
};

struct Call_Frame {
	size_t scope_depth;
	size_t block_reference;
	size_t arg_count;
	
	BC * bytecode;
	size_t bc_pointer;
	size_t bc_length;
	static Call_Frame create(Blocks * blocks, size_t block_reference, size_t arg_count)
	{
		Call_Frame frame;
		frame.block_reference = block_reference;

		frame.bytecode = blocks->retrieve_block(block_reference);
		frame.bc_pointer = 0;
		frame.bc_length = blocks->size_block(block_reference);
		
		frame.scope_depth = 1;
		frame.arg_count = arg_count;
		return frame;
	}
};

struct VM {
	Blocks * blocks;
	
	List<Value> stack;
	List<Scope*> scope_stack;
	
	List<Call_Frame> call_stack;
	
	void init(Blocks * blocks, size_t block_reference)
	{
		this->blocks = blocks;
		
		stack.alloc();
		
		call_stack.alloc();
		call_stack.push(Call_Frame::create(blocks, block_reference, 0));
		
		scope_stack.alloc();
		scope_stack.push(Scope::alloc_empty()); // Global scope
	}
	void destroy()
	{
		stack.dealloc();
		call_stack.dealloc();
		scope_stack.dealloc();
	}
	bool halted()
	{
		return call_stack.size == 0;
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
		return scope_stack[scope_stack.size - 1];
	}
	Scope * scope_at_offset(size_t offset)
	{
		return scope_stack[scope_stack.size - 1 - offset];
	}
	void mark_reachable()
	{
		for (int i = 0; i < stack.size; i++) {
			stack[i].gc_mark();
		}
	}
	void pop_scope()
	{
		auto scope = scope_stack.pop();
		scope->destroy();
		free(scope);
	}
	void return_function()
	{
		auto top_frame = &call_stack[call_stack.size - 1];
		// Save return value
		auto return_value = pop();
		// Pop arguments
		for (int i = 0; i < top_frame->arg_count; i++) {
			pop();
		}
		// Push return value back
		push(return_value);
		// Pop scopes
		for (int i = 0; i < top_frame->scope_depth; i++) {
			pop_scope();
		}
		call_stack.pop();
	}
	void step()
	{
		/* Manage call stack and VM halting */
		Call_Frame * frame;

		{
			if (halted()) {
				return;
			}

			auto top_frame = &call_stack[call_stack.size - 1];
			
			// If our call frame has come to an implicit end
			if (top_frame->bc_pointer >= top_frame->bc_length) {
				// HACK: If we're exiting from global scope, push a
				// dummy value to appease the assert up ahead.
				if (call_stack.size == 1) {
					push(Value::nothing());
				}
				
				// There should *always* be something pushed to the stack
				// at this point, because lambda bodies are expressions,
				// and expressions always terminate with a value having
				// been pushed to the stack.
				assert(stack.size > 0);
				
				assert(top_frame->scope_depth == 1); // An implicit end
												     // indicates only one
												     // level of scope
												     // depth.
				return_function();
				
				if (halted()) { // If we've just returned from global
								// scope, we're halted and should
								// return
					return;
				}
			}

			frame = &call_stack[call_stack.size - 1];
		}
		
		BC bc = frame->bytecode[frame->bc_pointer++];
		
		switch (bc.kind) {
		case BC_POP_AND_DISCARD: {
			pop();
		} break;
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
			// Recurse back through available scopes to find the var
			bool found_var = false;
			for (int i = 0; i < frame->scope_depth; i++) {
				bool success = scope_at_offset(i)->resolve_binding(symbol.symbol, &offset);
				if (success) {
					found_var = true;
					break;
				}
			}
			// Backup into global scope in case we didn't find it anywhere else
			if (!found_var) {
				found_var = scope_stack[0]->resolve_binding(symbol.symbol, &offset);
			}
			// If we STILL have nothing, it's not bound
			if (!found_var) {
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
		case BC_ADD_TWO: {
			auto b = pop();
			auto a = pop();
			push(Value::add(a, b));
		} break;
		case BC_SUB_TWO: {
			auto b = pop();
			auto a = pop();
			push(Value::subtract(a, b));
		} break;
		case BC_MUL_TWO: {
			auto b = pop();
			auto a = pop();
			push(Value::multiply(a, b));
		} break;
		case BC_DIV_TWO: {
			auto b = pop();
			auto a = pop();
			push(Value::divide(a, b));
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
		case BC_POP_AND_CALL_FUNCTION: {
			auto func = pop();
			func.assert_is(TYPE_FUNCTION);
			auto passed_arg_count = pop();
			passed_arg_count.assert_is(TYPE_INTEGER);
			if (passed_arg_count.integer != func.ref_function->parameter_count) {
				fatal("Function takes %d arguments; was passed %d",
					  func.ref_function->parameter_count,
					  passed_arg_count.integer);
			}

			call_stack.push(Call_Frame::create(blocks, func.ref_function->block_reference,
											   passed_arg_count.integer));
			scope_stack.push(Scope::alloc_empty());

			auto scope = current_scope();
			// Create bindings to pushed arguments
			for (int i = 0; i < passed_arg_count.integer; i++) {
				scope->create_binding(func.ref_function->parameters[i],
									  stack.size - i - 1);
			}
		} break;
		case BC_RETURN: {
			return_function();
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
		if (call_stack.size > 0) {
			auto frame = &call_stack[call_stack.size - 1];
			int index = frame->bc_pointer;
			if (index < frame->bc_length) {
				if (index > 0) {
					char * s = frame->bytecode[index - 1].to_string();
					printf("%s\n", s);
					free(s);
				} else {
					printf("POP_AND_CALL_FUNCTION\n"); // HACK
				}
			}
		} else {
			printf(".HALTED.\n");
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
		/*
		if (!halted()) {
			printf("%d scopes; %d reachable\n",
				   scope_stack.size,
				   call_stack[call_stack.size - 1].scope_depth);
				   }*/
		for (int i = scope_stack.size - 1; i >= 0; i--) {
			auto scope = scope_stack[i];
			assert(scope->symbols.size == scope->offsets.size);
			for (int j = 0; j < scope->symbols.size; j++) {
				printf("  %s: (%lu) ", scope->symbols[j], scope->offsets[j]);
				{
					char * s = stack[scope->offsets[j]].to_string();
					printf("%s\n", s);
					free(s);
				}
			}
			if (i > 0) {
				printf("      .\n");
			}
		}
		printf("-------------\n\n");
	}
};
