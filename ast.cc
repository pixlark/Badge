#define PRETTY_INDENT_MULT 4

struct Expr;

enum Stmt_Kind {
	STMT_LET,
	STMT_SET,
	STMT_PRINT,
	STMT_RETURN,
	STMT_EXPR,
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

struct Stmt_Return {
	Expr * expr;
	void destroy();
};

struct Stmt {
	Stmt_Kind kind;
	union {
		Stmt_Let let;
		Stmt_Set set;
		Stmt_Print print;
		Stmt_Return _return;
		Expr * expr;
	};
	static Stmt * with_kind(Stmt_Kind kind);
	void destroy();
};

/*
 * Expr
 */

enum Operator {
	OP_NEGATE,
	OP_ADD,
	OP_SUBTRACT,
	OP_MULTIPLY,
	OP_DIVIDE,
};

enum Expr_Kind {
	EXPR_INTEGER,
	EXPR_VARIABLE,
	EXPR_UNARY,
	EXPR_BINARY,
	EXPR_LAMBDA,
	EXPR_FUNCALL,
};

struct Expr_Unary {
	Operator op;
	Expr * expr;
	void destroy();
};

struct Expr_Binary {
	Expr * left;
	Operator op;
	Expr * right;
	void destroy();
};

struct Expr_Lambda {
	List<Symbol> parameters;
	List<Stmt*> body;
	void destroy();
};

struct Expr_Funcall {
	Expr * func;
	List<Expr*> args;
	void destroy();
};

struct Expr {
	Expr_Kind kind;
	union {
		int integer;
		Symbol variable;
		Expr_Unary unary;
		Expr_Binary binary;
		Expr_Lambda lambda;
		Expr_Funcall funcall;
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
		case EXPR_UNARY:
			unary.destroy();
			break;
		case EXPR_BINARY:
			binary.destroy();
			break;
		case EXPR_LAMBDA:
			lambda.destroy();
			break;
		case EXPR_FUNCALL:
			funcall.destroy();
			break;
		}
	}
};

void Expr_Unary::destroy()
{
	expr->destroy();
	free(expr);
}

void Expr_Binary::destroy()
{
	left->destroy();
	free(left);
	right->destroy();
	free(right);
}

void Expr_Lambda::destroy()
{
	parameters.dealloc();
	for (int i = 0; i < body.size; i++) {
		body[i]->destroy();
		free(body[i]);
	}
	body.dealloc();
}

void Expr_Funcall::destroy()
{
	func->destroy();
	free(func);
	for (int i = 0; i < args.size; i++) {
		args[i]->destroy();
		free(args[i]);
	}
	args.dealloc();
}

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

void Stmt_Return::destroy()
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
	case STMT_RETURN:
		_return.destroy();
		break;
	case STMT_EXPR:
		expr->destroy();
		free(expr);
		break;
	}
}
