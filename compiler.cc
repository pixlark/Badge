struct Compiler {
	List<BC> bytecode;
	Blocks * blocks;
	size_t block_reference;
	void init(Blocks * blocks)
	{
		bytecode.alloc();
		this->blocks = blocks;
		block_reference = blocks->make_block();
	}
	void finalize()
	{
		BC * final_bc = (BC*) malloc(sizeof(BC) * bytecode.size);
		for (int i = 0; i < bytecode.size; i++) {
			final_bc[i] = bytecode[i];
		}
		blocks->finalize_block(block_reference, final_bc, bytecode.size);
	}
	void destroy()
	{
		bytecode.dealloc();
	}
	void push(BC bc)
	{
		bytecode.push(bc);
	}	
	void compile_operator(Operator op)
	{
		switch (op) {
		case OP_NEGATE:
			fatal("Unimplemented");
			break;
		case OP_ADD:
			push(BC::create(BC_ADD));
			break;
		case OP_SUBTRACT:
			push(BC::create(BC_SUBTRACT));
			break;
		case OP_MULTIPLY:
			push(BC::create(BC_MULTIPLY));
			break;
		case OP_DIVIDE:
			push(BC::create(BC_DIVIDE));
			break;
		case OP_EQUAL:
			push(BC::create(BC_EQUAL));
			break;
		case OP_NOT_EQUAL:
			push(BC::create(BC_NOT_EQUAL));
			break;
		case OP_LESS_THAN:
			push(BC::create(BC_LESS_THAN));
			break;
		case OP_GREATER_THAN:
			push(BC::create(BC_GREATER_THAN));
			break;
		case OP_LESS_THAN_OR_EQUAL_TO:
			push(BC::create(BC_LESS_THAN_OR_EQUAL_TO));
			break;
		case OP_GREATER_THAN_OR_EQUAL_TO:
			push(BC::create(BC_GREATER_THAN_OR_EQUAL_TO));
			break;
		case OP_AND:
			push(BC::create(BC_AND));
			break;
		case OP_OR:
			push(BC::create(BC_OR));
			break;
		}
	}
	void compile_expr(Expr * expr)
	{
		switch (expr->kind) {
		case EXPR_NOTHING:
			push(BC::create(BC_LOAD_CONST,
							Value::nothing()));
			break;
		case EXPR_UNARY:
			compile_expr(expr->unary.expr);
			compile_operator(expr->unary.op);
			break;
		case EXPR_BINARY:
			compile_expr(expr->binary.left);
			compile_expr(expr->binary.right);
			compile_operator(expr->binary.op);
			break;
		case EXPR_INTEGER:
			push(BC::create(BC_LOAD_CONST,
							Value::raise(expr->integer)));
			break;
		case EXPR_VARIABLE:
			push(BC::create(BC_LOAD_CONST,
							Value::raise(expr->variable)));
			push(BC::create(BC_RESOLVE_BINDING));
			break;
		case EXPR_SCOPE: {
			auto body = expr->scope.body;
			auto terminator = expr->scope.terminator;
			push(BC::create(BC_ENTER_SCOPE));
			for (int i = 0; i < body.size; i++) {
				compile_stmt(body[i]);
			}
			if (terminator) {
				compile_expr(terminator);
			} else {
				push(BC::create(BC_LOAD_CONST,
								Value::nothing()));
			}
			push(BC::create(BC_EXIT_SCOPE));
		} break;
		case EXPR_LAMBDA: {
			auto params = expr->lambda.parameters;
			for (int i = 0; i < params.size; i++) {
				push(BC::create(BC_LOAD_CONST,
								Value::raise(params[i])));
			}
			push(BC::create(BC_LOAD_CONST,
							Value::raise(params.size)));
			Compiler compiler;
			compiler.init(blocks);
			/*
			auto body = expr->lambda.body;
			for (int i = 0; i < body.size; i++) {
				compiler.compile_stmt(body[i]);
				}*/
			compiler.compile_expr(expr->lambda.body);
			compiler.finalize();
			compiler.destroy();
			
			push(BC::create(BC_CONSTRUCT_FUNCTION,
							compiler.block_reference));
		} break;
		case EXPR_FUNCALL: {
			auto args = expr->funcall.args;
			// Args are pushed in reverse order
			for (int i = args.size - 1; i >= 0; i--) {
				compile_expr(args[i]);
			}
			push(BC::create(BC_LOAD_CONST,
							Value::raise(args.size)));
			compile_expr(expr->funcall.func);
			push(BC::create(BC_POP_AND_CALL_FUNCTION));
		} break;
		case EXPR_IF: {
			List<int> end_jumps;
			end_jumps.alloc();

			auto _if = expr->if_expr;
			assert(_if.conditions.size == _if.expressions.size);
			for (int i = 0; i < _if.conditions.size; i++) {
				compile_expr(_if.conditions[i]);
				push(BC::create(BC_NOT));
				push(BC::create(BC_POP_JUMP));
				int skip_pos = bytecode.size - 1;
				compile_expr(_if.expressions[i]);
				push(BC::create(BC_JUMP));
				end_jumps.push(bytecode.size - 1);
				bytecode[skip_pos].arg.integer = bytecode.size;
			}
			if (_if.else_expr) {
				compile_expr(_if.else_expr);
			} else {
				push(BC::create(BC_LOAD_CONST,
								Value::nothing()));
			}
			push(BC::create(BC_NOP));
			for (int i = 0; i < end_jumps.size; i++) {
				assert(bytecode[end_jumps[i]].kind == BC_JUMP);
				bytecode[end_jumps[i]].arg.integer = bytecode.size - 1;
			}
			
			end_jumps.dealloc();
		} break;
		}
	}
	void compile_stmt(Stmt * stmt)
	{
		switch (stmt->kind) {
		case STMT_LET:
			compile_expr(stmt->let.right);
			push(BC::create(BC_LOAD_CONST,
							Value::raise(stmt->let.left)));
			push(BC::create(BC_CREATE_BINDING));
			break;
		case STMT_SET:
			compile_expr(stmt->set.right);
			push(BC::create(BC_LOAD_CONST,
							Value::raise(stmt->set.left)));
			push(BC::create(BC_UPDATE_BINDING));
			break;
		case STMT_PRINT:
			compile_expr(stmt->print.expr);
			push(BC::create(BC_POP_AND_PRINT));
			break;
		case STMT_RETURN:
			compile_expr(stmt->_return.expr);
			push(BC::create(BC_RETURN));
			break;
		case STMT_EXPR:
			compile_expr(stmt->expr);
			push(BC::create(BC_POP_AND_DISCARD));
			break;
		}
	}
};
