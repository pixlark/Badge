#define PRETTY_INDENT_MULT 4

struct Expr;

enum Stmt_Kind {
	STMT_LET,
	STMT_SET,
	STMT_PRINT,
};

struct Stmt_Let {
	Symbol left;
	Expr * right;
	void destroy();
};

struct Stmt_Set {
	Symbol left;
	Expr * right;
	void destroy();
};

struct Stmt_Print {
	Expr * expr;
	void destroy();
};

struct Stmt {
	Stmt_Kind kind;
	union {
		Stmt_Let let;
		Stmt_Set set;
		Stmt_Print print;
	};
	static Stmt * with_kind(Stmt_Kind kind);
	void destroy();
	char * _to_string(int indent);
	char * to_string();
};

/*
 * Expr
 */

enum Expr_Kind {
	EXPR_INTEGER,
	EXPR_VARIABLE,
	EXPR_LAMBDA,
};

struct Expr_Lambda {
	List<Symbol> parameters;
	List<Stmt*> body;
};

struct Expr {
	Expr_Kind kind;
	union {
		int integer;
		Symbol variable;
		Expr_Lambda lambda;
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
		case EXPR_VARIABLE:
			break;
		case EXPR_LAMBDA:
			lambda.parameters.dealloc();
			for (int i = 0; i < lambda.body.size; i++) {
				lambda.body[i]->destroy();
				free(lambda.body[i]);
			}
			lambda.body.dealloc();
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
		case EXPR_VARIABLE: {
			builder.append(variable);
		} break;
		case EXPR_LAMBDA: {
			builder.append("lambda (");
			for (int i = 0; i < lambda.parameters.size; i++) {
				builder.append(lambda.parameters[i]);
				if (i < lambda.parameters.size - 1) {
					builder.append(", ");
				}
			}
			builder.append(") {\n");
			for (int i = 0; i < lambda.body.size; i++) {
				char * s = lambda.body[i]->_to_string(indent + 1);
				builder.append(s);
				free(s);
			}
			builder.append(" }\n");
		} break;
		}
		
		return builder.final_string();
	}
	char * to_string()
	{
		return this->_to_string(0);
	}
};

/*
 * Stmt Instances
 */


void Stmt_Let::destroy()
{
	right->destroy();
	free(right);
}

void Stmt_Set::destroy()
{
	right->destroy();
	free(right);
}

void Stmt_Print::destroy()
{
	expr->destroy();
	free(expr);
}

/*
 * Stmt
 */

Stmt * Stmt::with_kind(Stmt_Kind kind)
{
	Stmt * stmt = (Stmt*) malloc(sizeof(Stmt));
	stmt->kind = kind;
	return stmt;
}

void Stmt::destroy()
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
char * Stmt::_to_string(int indent)
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
char * Stmt::to_string()
{
	return this->_to_string(0);
}

