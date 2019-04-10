struct Compiler {
	GC_List<BC> bytecode;
	void init()
	{
		bytecode.make();
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
			auto body = expr->lambda.body;
			Compiler compiler;
			compiler.init();
			for (int i = 0; i < body.size; i++) {
				compiler.compile_stmt(body[i]);
			}
			Value_Bytecode * value_bytecode =
				(Value_Bytecode*) GC::alloc(sizeof(Value_Bytecode));
			value_bytecode->bytecode = compiler.bytecode;
			bytecode.push(BC::create(BC_LOAD_CONST,
									 Value::raise(value_bytecode)));
			bytecode.push(BC::create(BC_CONSTRUCT_FUNCTION));
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
		}
	}
};
