#define PRETTY_INDENT_MULT 4

enum Expr_Kind {
	EXPR_INTEGER,
};

struct Expr {
	Expr_Kind kind;
	union {
		int integer;
	};
	static Expr * with_kind(Expr_Kind kind)
	{
		Expr * expr = (Expr*) malloc(sizeof(Expr));
		expr->kind = kind;
		return expr;
	}
	void destroy()
	{
		switch (kind) {
		case EXPR_INTEGER:
			break;
		}
	}
	char * _to_string(int indent)
	{
		String_Builder builder;
		for (int i = 0; i < indent * PRETTY_INDENT_MULT; i++) {
			builder.append(" ");
		}

		switch (kind) {
		case EXPR_INTEGER: {
			char * s = itoa(integer);
			builder.append(s);
			free(s);
		} break;
		}
		
		return builder.final_string();
	}
	char * to_string()
	{
		return this->_to_string(0);
	}
};

enum Stmt_Kind {
	STMT_LET,
	STMT_SET,
	STMT_PRINT,
};

struct Stmt_Let {
	Symbol left;
	Expr * right;
	void destroy()
	{
		right->destroy();
		free(right);
	}
};

struct Stmt_Set {
	Symbol left;
	Expr * right;
	void destroy()
	{
		right->destroy();
		free(right);
	}
};

struct Stmt_Print {
	Expr * expr;
	void destroy()
	{
		expr->destroy();
		free(expr);
	}
};

struct Stmt {
	Stmt_Kind kind;
	union {
		Stmt_Let let;
		Stmt_Set set;
		Stmt_Print print;
	};
	static Stmt * with_kind(Stmt_Kind kind)
	{
		Stmt * stmt = (Stmt*) malloc(sizeof(Stmt));
		stmt->kind = kind;
		return stmt;
	}
	void destroy()
	{
		switch (kind) {
		case STMT_LET:
			let.destroy();
			break;
		case STMT_SET:
			set.destroy();
			break;
		case STMT_PRINT:
			print.destroy();
			break;
		}
	}
	char * _to_string(int indent)
	{
		String_Builder builder;
		for (int i = 0; i < indent * PRETTY_INDENT_MULT; i++) {
			builder.append(" ");
		}
		switch (kind) {
		case STMT_LET: {
			char buf[512];
			sprintf(buf, "let %s = \n", let.left);
			builder.append(buf);
			
			char * right_s = let.right->_to_string(indent + 1);
			builder.append(right_s);
			free(right_s);
		} break;
		case STMT_SET: {
			char buf[512];
			sprintf(buf, "set %s = \n", set.left);
			builder.append(buf);
			
			char * right_s = set.right->_to_string(indent + 1);
			builder.append(right_s);
			free(right_s);
		} break;
		case STMT_PRINT: {
			builder.append("print\n");
			char * expr_s = print.expr->_to_string(indent + 1);
			builder.append(expr_s);
			free(expr_s);
		} break;
		}
		return builder.final_string();
	}
	char * to_string()
	{
		return this->_to_string(0);
	}
};
