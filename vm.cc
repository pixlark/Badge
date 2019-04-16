struct Call_Frame {
	Environment * environment;
	size_t block_reference;
	size_t arg_count;
	size_t original_offset;
	
	BC * bytecode;
	size_t bc_pointer;
	size_t bc_length;
	static Call_Frame * alloc(Blocks * blocks, size_t block_reference,
							  size_t arg_count, size_t original_offset)
	{
		Call_Frame * frame = (Call_Frame*) GC::alloc(sizeof(Call_Frame));
		frame->environment = Environment::alloc();
		frame->block_reference = block_reference;

		frame->bytecode = blocks->retrieve_block(block_reference);
		frame->bc_pointer = 0;
		frame->bc_length = blocks->size_block(block_reference);
		
		frame->arg_count = arg_count;
		frame->original_offset = original_offset;
		return frame;
	}
	void gc_mark()
	{
		GC::mark_opaque(environment);
		environment->gc_mark();
	}
};

struct VM {
	Blocks * blocks;
	List<Value> stack;
	List<Call_Frame*> call_stack;
	
	void init(Blocks * blocks, size_t block_reference)
	{
		this->blocks = blocks;
		
		stack.alloc();
		
		call_stack.alloc();
		call_stack.push(Call_Frame::alloc(blocks, block_reference, 0, 0));
	}
	void destroy()
	{
		stack.dealloc();
		call_stack.dealloc();
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
	void mark_reachable()
	{
		for (int i = 0; i < call_stack.size; i++) {
			GC::mark_opaque(call_stack[i]);
			call_stack[i]->gc_mark();
		}
		for (int i = 0; i < stack.size; i++) {
			stack[i].gc_mark();
		}
	}
	void return_function()
	{
		auto top_frame = frame_reference();
		// Save return value
		auto return_value = pop();
		// Pop until reached original offset
		assert(stack.size >= top_frame->original_offset);
		while (stack.size > top_frame->original_offset) {
			pop();
		}
		// Pop arguments
		for (int i = 0; i < top_frame->arg_count; i++) {
			pop();
		}
		// Push return value back
		push(return_value);
		call_stack.pop();
	}
	Call_Frame * frame_reference()
	{
		assert(call_stack.size > 0);
		return call_stack[call_stack.size - 1];
	}
	size_t resolve_binding(Symbol symbol)
	{
		auto frame = frame_reference();
		size_t offset;
		if (frame->environment->resolve_binding(symbol, &offset)) {
			return offset;
		}
		auto global = call_stack[0];
		if (global->environment->resolve_binding(symbol, &offset)) {
			return offset;
		}
		fatal("Variable '%s' is not bound", symbol);
		assert(false); // @linter
	}
	void step()
	{
		if (halted()) {
			return;
		}
		
		/* Manage call stack and VM halting */
		// TODO(pixlark): Clean this mess up
		Call_Frame * frame;
		{
			frame = frame_reference();
			
			// If our call frame has come to an implicit end
			while (frame->bc_pointer >= frame->bc_length) {
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

				return_function();
				
				if (halted()) { // If we've just returned from global
								// scope, we're halted and should
								// return
					
					// First, clear out the stack so that the garbage
					// collector can clean everything up
					size_t remaining = stack.size;
					for (int i = 0; i < remaining; i++) {
						pop();
					}
					return;
				}
				
				// Update frame reference
				frame = frame_reference();
			}
		}
		
		BC bc = frame->bytecode[frame->bc_pointer++];
		
		switch (bc.kind) {
		case BC_NOP: break;
		case BC_POP_AND_DISCARD: {
			pop();
		} break;
		case BC_LOAD_CONST: {
			push(bc.arg.value);
		} break;
		case BC_CREATE_BINDING: {
			auto symbol = pop();
			symbol.assert_is(TYPE_SYMBOL);
			//bool success = current_scope()->create_binding(symbol.symbol, top_offset());
			bool success = frame->environment->create_binding(symbol.symbol, top_offset());
			if (!success) {
				fatal("Can't create new variable '%s' -- already bound in this scope!",
					  symbol.symbol);
			}
		} break;
		case BC_UPDATE_BINDING: {
			auto symbol = pop();
			symbol.assert_is(TYPE_SYMBOL);
			auto new_value = pop();
			auto offset = resolve_binding(symbol.symbol);
			assert(offset < stack.size);
			stack[offset] = new_value;
		} break;
		case BC_RESOLVE_BINDING: {
			auto symbol = pop();
			symbol.assert_is(TYPE_SYMBOL);
			auto offset = resolve_binding(symbol.symbol);
			assert(offset < stack.size);
			push(stack[offset]);
		} break;
		case BC_POP_AND_PRINT: {
			auto v = pop();
			char * s = v.to_string();
			printf("%s\n", s);
			free(s);
		} break;
		case BC_ADD: {
			auto b = pop();
			auto a = pop();
			push(Value::add(a, b));
		} break;
		case BC_SUBTRACT: {
			auto b = pop();
			auto a = pop();
			push(Value::subtract(a, b));
		} break;
		case BC_MULTIPLY: {
			auto b = pop();
			auto a = pop();
			push(Value::multiply(a, b));
		} break;
		case BC_DIVIDE: {
			auto b = pop();
			auto a = pop();
			push(Value::divide(a, b));
		} break;
		case BC_EQUAL: {
			auto b = pop();
			auto a = pop();
			if (Value::equal(a, b)) {
				push(Value::raise(1));
			} else {
				push(Value::nothing());
			}
		} break;
		case BC_NOT_EQUAL: {
			auto b = pop();
			auto a = pop();
			if (Value::equal(a, b)) {
				push(Value::nothing());
			} else {
				push(Value::raise(1));
			}
		} break;
		case BC_AND: {
			auto b = pop();
			auto a = pop();
			push(Value::_and(a, b));
		} break;
		case BC_OR: {
			auto b = pop();
			auto a = pop();
			push(Value::_or(a, b));
		} break;
		case BC_NOT: {
			auto a = pop();
			if (a.type == TYPE_NOTHING) {
				push(Value::raise(1));
			} else {
				push(Value::nothing());
			}
		} break;
		case BC_CONSTRUCT_FUNCTION: {
			auto count = pop();
			count.assert_is(TYPE_INTEGER);
			
			Function * func = (Function*) GC::alloc(sizeof(Function));
			func->block_reference = bc.arg.block_reference;

			// Insert parameters
			func->parameter_count = count.integer;
			func->parameters = (Symbol*) GC::alloc(sizeof(Symbol) * count.integer);
			for (int i = 0; i < count.integer; i++) {
				auto it = pop();
				it.assert_is(TYPE_SYMBOL);
				func->parameters[count.integer - i - 1] = it.symbol;
			}

			// Close over local environment
			func->closure = frame->environment;
			
			// Create and push value
			Value value = Value::create(TYPE_FUNCTION);
			value.ref_function = func;
			push(value);
		} break;
		case BC_POP_AND_CALL_FUNCTION: {
			auto func_val = pop();
			func_val.assert_is(TYPE_FUNCTION);
			auto func = func_val.ref_function;
			auto passed_arg_count = pop();
			passed_arg_count.assert_is(TYPE_INTEGER);
			if (passed_arg_count.integer != func->parameter_count) {
				fatal("Function takes %d arguments; was passed %d",
					  func->parameter_count,
					  passed_arg_count.integer);
			}
			call_stack.push(Call_Frame::alloc(blocks, func->block_reference,
											  func->parameter_count, stack.size));
			
			frame = frame_reference();
			auto env = frame->environment;
			
			// Create bindings to pushed arguments
			for (int i = 0; i < passed_arg_count.integer; i++) {
				env->create_binding(func->parameters[i],
									stack.size - i - 1);
			}
			/*
			// Bind closured values
			for (int i = 0; i < func->closure.size; i++) {
				push(func->closure.values[i]);
				env->create_binding(func->closure.names[i],
									stack.size - 1);
									}*/
		} break;
		case BC_RETURN: {
			// WARNING: `frame` invalidated here! Don't use it!
			return_function();
		} break;
		case BC_POP_JUMP: {
			auto a = pop();
			if (a.type == TYPE_NOTHING) {
				break;
			}
		} /* FALLTHROUGH */
		case BC_JUMP: {
			frame->bc_pointer = bc.arg.integer;
		} break;
		}
		
		GC::unmark_all();
		mark_reachable();
		GC::free_unmarked();
	}
	// TODO(pixlark): This function is a right mess. Clean this up at
	// some point.
	void print_debug_info()
	{
		printf("--- Frame ---\n");
		const int width = 13;
		if (call_stack.size > 0) {
			auto frame = frame_reference();
			int index = frame->bc_pointer;
			if (index < frame->bc_length) {
				if (index > 0) {
					char * s = frame->bytecode[index - 1].to_string();
					printf("%s\n", s);
					free(s);
				} else {
					printf("POP_AND_CALL_FUNCTION\n"); // HACK
				}
			} else {
				char * s = frame->bytecode[index - 1].to_string();
				printf("%s\n", s);
				free(s);
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
		for (int i = call_stack.size - 1; i >= 0; i--) {
			auto frame = call_stack[i];
			auto env = frame->environment;
			for (int j = 0; j < env->names.size; j++) {
				auto sym = env->names[j];
				auto o = env->offsets[j];
				char * s = stack[o].to_string();
				printf("%s: (%zu) %s\n", sym, o, s);
				free(s);
			}
			if (i > 0) {
				printf("      .\n");
			}
		}
		printf("-------------\n\n");
	}
};
