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
	void compile_expr(Expr * expr)
	{
		switch (expr->kind) {
		case EXPR_INTEGER:
			bytecode.push(BC::create(BC_LOAD_CONST,
									 Value::raise(expr->integer)));
			break;
		case EXPR_VARIABLE:
			bytecode.push(BC::create(BC_LOAD_CONST,
									 Value::raise(expr->variable)));
			bytecode.push(BC::create(BC_RESOLVE_BINDING));
			break;
		case EXPR_LAMBDA: {
			auto params = expr->lambda.parameters;
			for (int i = 0; i < params.size; i++) {
				bytecode.push(BC::create(BC_LOAD_CONST,
										 Value::raise(params[i])));
			}
			bytecode.push(BC::create(BC_LOAD_CONST,
									 Value::raise(params.size)));
			Compiler compiler;
			compiler.init(blocks);
			auto body = expr->lambda.body;
			for (int i = 0; i < body.size; i++) {
				compiler.compile_stmt(body[i]);
			}
			compiler.finalize();
			compiler.destroy();
			
			bytecode.push(BC::create(BC_CONSTRUCT_FUNCTION,
									 compiler.block_reference));
		} break;
		case EXPR_FUNCALL: {
			auto args = expr->funcall.args;
			// Args are pushed in reverse order
			for (int i = args.size - 1; i >= 0; i--) {
				compile_expr(args[i]);
			}
			bytecode.push(BC::create(BC_LOAD_CONST,
									 Value::raise(args.size)));
			compile_expr(expr->funcall.func);
			bytecode.push(BC::create(BC_POP_AND_CALL_FUNCTION));
		} break;
		}
	}
	void compile_stmt(Stmt * stmt)
	{
		switch (stmt->kind) {
		case STMT_LET:
			compile_expr(stmt->let.right);
			bytecode.push(BC::create(BC_LOAD_CONST,
									 Value::raise(stmt->let.left)));
			bytecode.push(BC::create(BC_CREATE_BINDING));
			break;
		case STMT_SET:
			compile_expr(stmt->set.right);
			bytecode.push(BC::create(BC_LOAD_CONST,
									 Value::raise(stmt->set.left)));
			bytecode.push(BC::create(BC_UPDATE_BINDING));
			break;
		case STMT_PRINT:
			compile_expr(stmt->print.expr);
			bytecode.push(BC::create(BC_POP_AND_PRINT));
			break;
		case STMT_RETURN:
			compile_expr(stmt->_return.expr);
			bytecode.push(BC::create(BC_RETURN));
			break;
		case STMT_EXPR:
			compile_expr(stmt->expr);
			bytecode.push(BC::create(BC_POP_AND_DISCARD));
			break;
		}
	}
};
