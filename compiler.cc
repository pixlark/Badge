struct Compiler {
	List<BC> bytecode;
	void init()
	{
		bytecode.alloc();
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
