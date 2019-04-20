struct Call_Frame {
	Environment * environment;
	size_t block_reference;
	
	BC * bytecode;
	size_t bc_pointer;
	size_t bc_length;
	static Call_Frame * alloc(Blocks * blocks, size_t block_reference, Environment * closure)
	{
		Call_Frame * frame = (Call_Frame*) GC::alloc(sizeof(Call_Frame));
		frame->environment = Environment::alloc();
		frame->environment->next_env = closure;
		frame->block_reference = block_reference;

		frame->bytecode = blocks->retrieve_block(block_reference);
		frame->bc_pointer = 0;
		frame->bc_length = blocks->size_block(block_reference);
		return frame;
	}
	void gc_mark()
	{
		if (!GC::is_marked_opaque(environment)) {
			GC::mark_opaque(environment);
			environment->gc_mark();
		}
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
		call_stack.push(Call_Frame::alloc(blocks, block_reference, NULL));
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
		call_stack.pop();
	}
	Call_Frame * frame_reference()
	{
		assert(call_stack.size > 0);
		return call_stack[call_stack.size - 1];
	}
	void create_binding(Symbol symbol, Value value)
	{
		auto frame = frame_reference();
		bool success = frame->environment->create_binding(symbol, value);
		if (!success) {
			fatal("Can't create new variable '%s' -- already bound in this scope!",
				  symbol);
		}
	}
	Value resolve_binding(Symbol symbol)
	{
		auto frame = frame_reference();
		Value value;
		if (frame->environment->resolve_binding(symbol, &value)) {
			return value;
		}
		// TODO(pixlark): Make it so that every environment implicity
		// links to the global environment, thus removing the need for
		// this call frame?
		auto global = call_stack[0];
		if (global->environment->resolve_binding(symbol, &value)) {
			return value;
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
			auto value = pop();
			create_binding(symbol.symbol, value);
		} break;
		case BC_UPDATE_BINDING: {
			auto symbol = pop();
			symbol.assert_is(TYPE_SYMBOL);
			auto value = pop();
			frame->environment->update_binding(symbol.symbol, value);
		} break;
		case BC_RESOLVE_BINDING: {
			auto symbol = pop();
			symbol.assert_is(TYPE_SYMBOL);
			auto value = resolve_binding(symbol.symbol);
			push(value);
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
			push(Value::raise_bool(Value::equal(a, b)));
		} break;
		case BC_NOT_EQUAL: {
			auto b = pop();
			auto a = pop();
			push(Value::raise_bool(!Value::equal(a, b)));
		} break;
		case BC_GREATER_THAN: {
			auto b = pop();
			auto a = pop();
			push(Value::raise_bool(Value::greater_than(a, b)));
		} break;
		case BC_LESS_THAN: {
			auto b = pop();
			auto a = pop();
			push(Value::raise_bool(Value::less_than(a, b)));
		} break;
		case BC_GREATER_THAN_OR_EQUAL_TO: {
			auto b = pop();
			auto a = pop();
			push(Value::raise_bool(Value::greater_than_or_equal_to(a, b)));
		} break;
		case BC_LESS_THAN_OR_EQUAL_TO: {
			auto b = pop();
			auto a = pop();
			push(Value::raise_bool(Value::less_than_or_equal_to(a, b)));
		} break;
		case BC_AND: {
			auto b = pop();
			auto a = pop();
			push(Value::raise_bool(Value::_and(a, b)));
		} break;
		case BC_OR: {
			auto b = pop();
			auto a = pop();
			push(Value::raise_bool(Value::_or(a, b)));
		} break;
		case BC_NOT: {
			auto a = pop();
			push(Value::raise_bool(!a.truthy()));
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
			if (func_val.is(TYPE_FFI)) {
				// If this is an FFI function, override everything and just do an FFI
				auto ffi = func_val.ffi;
				auto passed_arg_count = pop();
				passed_arg_count.assert_is(TYPE_INTEGER);
				if (passed_arg_count.integer != ffi->arg_count) {
					fatal("Function takes %d arguments; was passed %d",
						  ffi->arg_count,
						  passed_arg_count.integer);
				}
				Value * args = (Value*) malloc(sizeof(Value) * ffi->arg_count);
				for (int i = 0; i < ffi->arg_count; i++) {
					args[i] = pop();
				}
				push((ffi->funcptr)(args));
				free(args);
			} else {
				// Otherwise, this is a normal function
				func_val.assert_is(TYPE_FUNCTION);
				auto func = func_val.ref_function;
				auto passed_arg_count = pop();
				passed_arg_count.assert_is(TYPE_INTEGER);
				if (passed_arg_count.integer != func->parameter_count) {
					fatal("Function takes %d arguments; was passed %d",
						  func->parameter_count,
						  passed_arg_count.integer);
				}
				call_stack.push(Call_Frame::alloc(blocks, func->block_reference, func->closure));
			
				// Create bindings to pushed arguments
				for (int i = 0; i < passed_arg_count.integer; i++) {
					auto value = pop();
					create_binding(func->parameters[i], value);
				}
			}
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
		case BC_ENTER_SCOPE: {
			auto new_env = Environment::alloc();
			new_env->next_env = frame->environment;
			frame->environment = new_env;
		} break;
		case BC_EXIT_SCOPE: {
			frame->environment = frame->environment->next_env;
		} break;
		}

		#if COLLECTION
		GC::unmark_all();
		mark_reachable();
		GC::free_unmarked();
		#endif
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
			int index = 0;
			while (env) {
				for (int j = 0; j < env->names.size; j++) {
					auto sym = env->names[j];
					Value val;
					env->resolve_binding(sym, &val);
					char * s = val.to_string();
					printf("%s: %s\n", sym, s);
					free(s);
				}
				env = env->next_env;
				index++;
			}
			if (i > 0) {
				printf("      .\n");
			}
		}
		printf("-------------\n\n");
	}
};
