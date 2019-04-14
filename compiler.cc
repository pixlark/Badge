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
			push(BC::create(BC_ADD_TWO));
			break;
		case OP_SUBTRACT:
			push(BC::create(BC_SUB_TWO));
			break;
		case OP_MULTIPLY:
			push(BC::create(BC_MUL_TWO));
			break;
		case OP_DIVIDE:
			push(BC::create(BC_DIV_TWO));
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
			for (int i = 0; i < body.size; i++) {
				compile_stmt(body[i]);
			}
			if (terminator) {
				compile_expr(terminator);
			} else {
				push(BC::create(BC_LOAD_CONST,
								Value::nothing()));
			}
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
