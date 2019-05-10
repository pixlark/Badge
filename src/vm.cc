struct Call_Frame {
	Function * origin;
	
	Environment * environment;
	size_t block_reference;
	
	BC * bytecode;
	size_t bc_pointer;
	size_t bc_length;

	List<int> body_stack;
	/* A note on allocation:
	 *  Call frames themselves are managed manually through
	 *  malloc/free. However, some components need to be garbage
	 *  collected. These are marked with the gc_mark function.
	 */
	static Call_Frame * alloc(Blocks * blocks, size_t block_reference,
							  Function * origin, Environment * closure)
	{
		Call_Frame * frame = (Call_Frame*) malloc(sizeof(Call_Frame));
		frame->origin = origin;
		frame->environment = Environment::alloc();
		frame->environment->next_env = closure;
		frame->block_reference = block_reference;

		frame->bytecode = blocks->retrieve_block(block_reference);
		frame->bc_pointer = 0;
		frame->bc_length = blocks->size_block(block_reference);

		frame->body_stack.alloc();
		return frame;
	}
	void gc_mark()
	{
		if (origin) {
			GC::mark_opaque(origin);
			origin->gc_mark();
		}
		if (!GC::is_marked_opaque(environment)) {
			GC::mark_opaque(environment);
			environment->gc_mark();
		}
	}
	void destroy()
	{
		body_stack.dealloc();
	}
};

enum VM_Response {
	VM_OK,
	VM_HALTED,
	VM_SWITCH,
};

struct Export {
	Symbol symbol;
	Assoc_Ptr assoc;
};

struct VM {
	Blocks * blocks;
	Environment * export_scope;
	List<Export> export_queue;
	List<Value> stack;
	List<Call_Frame*> call_stack;
	Assoc_Ptr current_assoc;
	size_t block_reference_to_push;
	
	void init(Blocks * blocks, Environment * export_scope, size_t block_reference)
	{
		this->blocks = blocks;
		this->export_scope = export_scope;

		export_queue.alloc();
		stack.alloc();
		
		call_stack.alloc();
		call_stack.push(Call_Frame::alloc(blocks, block_reference, NULL, NULL));
	}
	void error(const char * fmt, ...) {
		va_list args;
		va_start(args, fmt);
		v_fatal_assoc(current_assoc, fmt, args);
		va_end(args);
	}
	void destroy()
	{
		export_queue.dealloc();
		stack.dealloc();
		// The call stack should be empty if we're destructing
		assert(call_stack.size == 0);
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
	int pop_integer()
	{
		auto val = pop();
		val.assert_is(TYPE_INTEGER);
		return val.integer;
	}
	Symbol pop_symbol()
	{
		auto val = pop();
		val.assert_is(TYPE_SYMBOL);
		return val.symbol;
	}
	size_t top_offset()
	{
		return stack.size - 1;
	}
	void mark_reachable()
	{
		GC::mark_opaque(export_scope);
		export_scope->gc_mark();
		for (int i = 0; i < call_stack.size; i++) {
			call_stack[i]->gc_mark();
		}
		for (int i = 0; i < stack.size; i++) {
			stack[i].gc_mark();
		}
	}
	void return_function()
	{
		auto frame = call_stack.pop();
		frame->destroy();
		free(frame);
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
			error("Can't create new variable '%s' -- already bound in this scope!",
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
		// Finally, check in our export scope
		if (export_scope->resolve_binding(symbol, &value)) {
			return value;
		}
		error("Variable '%s' is not bound", symbol);
		assert(false); // @linter
	}
	void do_halting_tasks()
	{
		// Push exported names to export_scope
		for (int i = 0; i < export_queue.size; i++) {
			auto _export = export_queue[i];
			current_assoc = _export.assoc;
			auto val = resolve_binding(_export.symbol);
			export_scope->create_binding(_export.symbol, val);
		}
		// Clear out the stack so that the garbage
		// collector can clean everything up
		size_t remaining = stack.size;
		for (int i = 0; i < remaining; i++) {
			pop();
		}
	}
	VM_Response step()
	{	
		if (halted()) {
			return VM_HALTED;
		}
		
		/* Manage call stack and VM halting */
		Call_Frame * frame;
		{
			frame = frame_reference();
			
			// If our call frame has come to an implicit end
			while (frame->bc_pointer >= frame->bc_length) {
				assert(stack.size > 0);

				if (call_stack.size == 1) {
					// If we're about to return from global scope, we
					// need to do some special stuff
					do_halting_tasks();
					return_function();
					return VM_HALTED;
				}
				return_function();
				
				// Update frame reference
				frame = frame_reference();
			}
		}
		
		BC bc = frame->bytecode[frame->bc_pointer++];
		current_assoc = bc.assoc;
		
		switch (bc.kind) {
		case BC_NOP: break;
		case BC_POP_AND_DISCARD: {
			pop();
		} break;
		case BC_LOAD_CONST: {
			push(bc.arg.value);
		} break;
		case BC_DUPLICATE: {
			push(stack[stack.size - 1]);
		} break;
		case BC_CREATE_BINDING: {
			auto symbol = pop_symbol();
			auto value = pop();
			create_binding(symbol, value);
		} break;
		case BC_UPDATE_BINDING: {
			auto symbol = pop_symbol();
			auto value = pop();
			if (!frame->environment->update_binding(symbol, value)) {
				error("Tried to set unbound variable '%s'", symbol);
			}
		} break;
		case BC_RESOLVE_BINDING: {
			auto symbol = pop_symbol();
			auto value = resolve_binding(symbol);
			push(value);
		} break;
		case BC_ADD: {
			auto b = pop();
			auto a = pop();
			push(Value::add(a, b, bc.assoc));
		} break;
		case BC_SUBTRACT: {
			auto b = pop();
			auto a = pop();
			push(Value::subtract(a, b, bc.assoc));
		} break;
		case BC_MULTIPLY: {
			auto b = pop();
			auto a = pop();
			push(Value::multiply(a, b, bc.assoc));
		} break;
		case BC_DIVIDE: {
			auto b = pop();
			auto a = pop();
			push(Value::divide(a, b, bc.assoc));
		} break;
		case BC_NEGATE: {
			auto a = pop();
			push(Value::subtract(Value::raise(0), a, bc.assoc));
		} break;
		case BC_EQUAL: {
			auto b = pop();
			auto a = pop();
			push(Value::raise_bool(Value::equal(a, b, bc.assoc)));
		} break;
		case BC_NOT_EQUAL: {
			auto b = pop();
			auto a = pop();
			push(Value::raise_bool(!Value::equal(a, b, bc.assoc)));
		} break;
		case BC_GREATER_THAN: {
			auto b = pop();
			auto a = pop();
			push(Value::raise_bool(Value::greater_than(a, b, bc.assoc)));
		} break;
		case BC_LESS_THAN: {
			auto b = pop();
			auto a = pop();
			push(Value::raise_bool(Value::less_than(a, b, bc.assoc)));
		} break;
		case BC_GREATER_THAN_OR_EQUAL_TO: {
			auto b = pop();
			auto a = pop();
			push(Value::raise_bool(Value::greater_than_or_equal_to(a, b, bc.assoc)));
		} break;
		case BC_LESS_THAN_OR_EQUAL_TO: {
			auto b = pop();
			auto a = pop();
			push(Value::raise_bool(Value::less_than_or_equal_to(a, b, bc.assoc)));
		} break;
		case BC_AND: {
			auto b = pop();
			auto a = pop();
			push(Value::raise_bool(Value::_and(a, b, bc.assoc)));
		} break;
		case BC_OR: {
			auto b = pop();
			auto a = pop();
			push(Value::raise_bool(Value::_or(a, b, bc.assoc)));
		} break;
		case BC_NOT: {
			auto a = pop();
			push(Value::raise_bool(!a.truthy()));
		} break;
		case BC_CONSTRUCT_FUNCTION: {
			auto count = pop_integer();
			
			Function * func = (Function*) GC::alloc(sizeof(Function));
			func->block_reference = bc.arg.block_reference;

			// Insert parameters
			func->parameter_count = count;
			func->parameters = (Symbol*) GC::alloc(sizeof(Symbol) * count);
			for (int i = 0; i < count; i++) {
				auto it = pop_symbol();
				func->parameters[count - i - 1] = it;
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
			if (func_val.is(TYPE_BUILTIN)) {
				// If this is a builtin function, override everything and just do a builtin call
				auto builtin = func_val.builtin;
				auto passed_arg_count = pop_integer();
				if (passed_arg_count != builtin->arg_count) {
					error("Function takes %d arguments; was passed %d",
						  builtin->arg_count,
						  passed_arg_count);
				}
				Value * args = (Value*) malloc(sizeof(Value) * builtin->arg_count);
				defer { free(args); };
				for (int i = 0; i < builtin->arg_count; i++) {
					args[i] = pop();
				}
				push((builtin->funcptr)(args));
			} else if (func_val.is(TYPE_CONSTRUCTOR)) {
				auto ctor = func_val.ref_constructor;
				auto object = (Object*) GC::alloc(sizeof(Object));
				object->fields.alloc(symbol_comparator);

				auto passed_arg_count = pop_integer();
				if (passed_arg_count != ctor->field_count) {
					error("Constructor has %d fields; was passed %d",
						  ctor->field_count,
						  passed_arg_count);
				}

				for (int i = 0; i < ctor->field_count; i++) {
					auto val = pop();
					auto symbol = ctor->fields[i];
					object->fields.add(symbol, val);
				}

				auto val = Value::create(TYPE_OBJECT);
				val.ref_object = object;
				push(val);
			} else {
				// Otherwise, this is a normal function
				func_val.assert_is(TYPE_FUNCTION);
				auto func = func_val.ref_function;
				auto passed_arg_count = pop_integer();
				if (passed_arg_count != func->parameter_count) {
					error("Function takes %d arguments; was passed %d",
						  func->parameter_count,
						  passed_arg_count);
				}

				#if TAIL_CALL_OPTIMIZATION
				bool tail_call = false;
				int i = frame->bc_pointer;
				// Push past any non-interfering instructions
				while (true) {
					//while (frame->bytecode[i++].kind == BC_EXIT_SCOPE) {
					if (i >= frame->bc_length ||
						frame->bytecode[i].kind == BC_RETURN) {
						tail_call = true;
						break;
					}
					if (!frame->bytecode[i++].no_interference_with_tail_calls()) {
						break;
					}
				}
				if (tail_call) {
					call_stack.pop();
				}
				#endif

				// Create our new call frame
				call_stack.push(Call_Frame::alloc(blocks, func->block_reference,
												  func, func->closure));
			
				// Create bindings to pushed arguments
				for (int i = 0; i < passed_arg_count; i++) {
					auto value = pop();
					create_binding(func->parameters[i], value);
				}
			}
		} break;
		case BC_RETURN: {
			// WARNING: `frame` invalidated here! Don't use it!
			return_function();
		} break;
		case BC_THIS_FUNCTION: {
			auto func = Value::create(TYPE_FUNCTION);
			if (!frame->origin) {
				error("Invalid use of this -- not in a function!");
			}
			func.ref_function = frame->origin;
			push(func);
		} break;
		case BC_SYMBOL_TO_STRING: {
			auto symbol = pop_symbol();
			auto string = (String*) GC::alloc(sizeof(String));
			string->length = strlen(symbol);
			string->string = (char*) GC::alloc(sizeof(char) * string->length);
			strncpy(string->string, symbol, string->length);
			auto value = Value::create(TYPE_STRING);
			value.ref_string = string;
			push(value);
		} break;
		case BC_JUMP: {
			frame->bc_pointer = bc.arg.integer;
		} break;
		case BC_POP_JUMP: {
			auto a = pop();
			if (a.type != TYPE_NOTHING) {
				frame->bc_pointer = bc.arg.integer;
			}
		} break;
		case BC_ENTER_SCOPE: {
			auto new_env = Environment::alloc();
			new_env->next_env = frame->environment;
			frame->environment = new_env;
		} break;
		case BC_EXIT_SCOPE: {
			frame->environment = frame->environment->next_env;
		} break;
		case BC_CONSTRUCT_CONSTRUCTOR: {
			auto count = pop_integer();
			auto ctor = (Constructor*) GC::alloc(sizeof(Constructor));
			ctor->fields = (Symbol*) GC::alloc(sizeof(Symbol) * count);
			ctor->field_count = count;
			for (int i = 0; i < count; i++) {
				ctor->fields[i] = pop_symbol();
			}
			auto val = Value::create(TYPE_CONSTRUCTOR);
			val.ref_constructor = ctor;
			push(val);
		} break;
		case BC_RESOLVE_FIELD: {
			auto symbol = pop_symbol();
			auto obj_val = pop();
			if (!obj_val.is(TYPE_OBJECT)) {
				error("Cannot access field of non-object");
			}
			auto obj = obj_val.ref_object;
			if (!obj->fields.bound(symbol)) {
				error("No such field %s on object", symbol);
			}
			auto resolved = obj->fields.lookup(symbol);
			push(resolved);
		} break;
		case BC_UPDATE_FIELD: {
			auto symbol = pop_symbol();
			auto obj_val = pop();
			if (!obj_val.is(TYPE_OBJECT)) {
				error("Cannot access field of non-object");
			}
			auto obj = obj_val.ref_object;
			if (!obj->fields.bound(symbol)) {
				error("No such field %s on object", symbol);
			}
			auto val = pop();
			obj->fields.update(symbol, val);
		} break;
		case BC_PUSH_BODY: {
			frame->body_stack.push(bc.arg.integer);
		} break;
		case BC_BREAK_BODY: {
			if (frame->body_stack.size == 0) {
				error("Nothing to break out of");
			}
			int exit_pos = frame->body_stack.pop();
			frame->bc_pointer = exit_pos;
		} break;
		case BC_RUN_FILE_UNIT: {
			block_reference_to_push = pop_integer();
			push(Value::nothing());
			return VM_SWITCH;
		} break;
		case BC_EXPORT_SYMBOL: {
			auto symbol = pop_symbol();
			export_queue.push((Export) { symbol, bc.assoc });
			push(Value::nothing());
		} break;
		}

		return VM_OK;
	}
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
					defer { free(s); };
					printf("%s\n", s);
				} else {
					printf("POP_AND_CALL_FUNCTION\n"); // HACK
				}
			} else {
				char * s = frame->bytecode[index - 1].to_string();
				defer { free(s); };
				printf("%s\n", s);
			}
		} else {
			printf(".HALTED.\n");
		}
		printf(".............\n");
		printf("    Stack\n");
		for (int i = top_offset(); i >= 0; i--) {
			char * s = stack[i].to_string();
			defer { free(s); };
			size_t len = strlen(s);

			if (len < width) {
				int spacing = (width - len) / 2;
				for (int i = 0; i < spacing; i++) {
					printf(" ");
				}
			}

			printf("%s\n", s);
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
					defer { free(s); };
					printf("%s: %s\n", sym, s);
				}
				env = env->next_env;
				index++;
			}
			if (i > 0) {
				printf("      .\n");
			}
		}
		/*
		printf(".............\n");
		{
			printf("Bodies: ");
			auto frame = frame_reference();
			for (int i = 0; i < frame->body_stack.size; i++) {
				printf("%d ", frame->body_stack[i]);
			}
			printf("\n");
			}*/
		printf("-------------\n\n");
	}
};
