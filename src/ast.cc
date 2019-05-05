#define PRETTY_INDENT_MULT 4

struct Expr;

enum Stmt_Kind {
	STMT_LET,
	STMT_SET,
	STMT_RETURN,
	STMT_EXPR,
	STMT_BREAK,
};

struct Stmt_Let {
	Symbol left;
	Expr * right;
	void destroy();
};

struct Stmt_Set {
	//Symbol left;
	Expr * left;
	Expr * right;
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
		Stmt_Return _return;
		Expr * expr;
		Expr * _break;
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
	OP_EQUAL,
	OP_NOT_EQUAL,
	OP_LESS_THAN,
	OP_GREATER_THAN,
	OP_LESS_THAN_OR_EQUAL_TO,
	OP_GREATER_THAN_OR_EQUAL_TO,
	OP_AND,
	OP_OR,
};

enum Expr_Kind {
	EXPR_NOTHING,
	EXPR_INTEGER,
	EXPR_STRING,
	EXPR_VARIABLE,
	EXPR_UNARY,
	EXPR_BINARY,
	EXPR_SCOPE,
	EXPR_LAMBDA,
	EXPR_FUNCALL,
	EXPR_IF,
	EXPR_DIRECTIVE,
	EXPR_THIS,
	EXPR_FIELD,
	EXPR_LOOP,
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

struct Expr_Scope {
	List<Stmt*> body;
	Expr * terminator;
	void destroy();
};

struct Expr_Lambda {
	List<Symbol> parameters;
	Expr * body;
	//List<Stmt*> body;
	void destroy();
};

struct Expr_Funcall {
	Expr * func;
	List<Expr*> args;
	void destroy();
};

struct Expr_If {
	List<Expr*> conditions;
	List<Expr*> expressions;
	Expr * else_expr;
	void destroy();
};

struct Expr_Directive {
	Symbol name;
	List<Expr*> arguments;
	void destroy();
};

struct Expr_Struct {
	List<Symbol> fields;
	void destroy();
};

struct Expr_Field {
	Expr * left;
	Symbol right;
	void destroy();
};

struct Expr_Loop {
	Expr * body;
	void destroy();
};

struct Expr {
	Expr_Kind kind;
	union {
		int integer;
		Symbol variable;
		Symbol string;
		Expr_Unary unary;
		Expr_Binary binary;
		Expr_Scope scope;
		Expr_Lambda lambda;
		Expr_Funcall funcall;
		Expr_If if_expr;
		Expr_Directive directive;
		Expr_Field field;
		Expr_Loop loop;
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
		case EXPR_NOTHING:
			break;
		case EXPR_INTEGER:
			break;
		case EXPR_STRING:
			break;
		case EXPR_VARIABLE:
			break;
		case EXPR_UNARY:
			unary.destroy();
			break;
		case EXPR_BINARY:
			binary.destroy();
			break;
		case EXPR_SCOPE:
			scope.destroy();
			break;
		case EXPR_LAMBDA:
			lambda.destroy();
			break;
		case EXPR_FUNCALL:
			funcall.destroy();
			break;
		case EXPR_IF:
			if_expr.destroy();
			break;
		case EXPR_DIRECTIVE:
			directive.destroy();
			break;
		case EXPR_THIS:
			break;
		case EXPR_FIELD:
			field.destroy();
			break;
		case EXPR_LOOP:
			loop.destroy();
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

void Expr_Scope::destroy()
{
	for (int i = 0; i < body.size; i++) {
		body[i]->destroy();
		free(body[i]);
	}
	body.dealloc();
	if (terminator) {
		terminator->destroy();
		free(terminator);
	}
}

void Expr_Lambda::destroy()
{
	parameters.dealloc();
	body->destroy();
	free(body);
	/*
	for (int i = 0; i < body.size; i++) {
		body[i]->destroy();
		free(body[i]);
	}
	body.dealloc();*/
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

void Expr_If::destroy()
{
	for (int i = 0; i < conditions.size; i++) {
		conditions[i]->destroy();
		free(conditions[i]);
	}
	conditions.dealloc();
	
	for (int i = 0; i < expressions.size; i++) {
		expressions[i]->destroy();
		free(expressions[i]);
	}
	expressions.dealloc();

	if (else_expr) {
		else_expr->destroy();
		free(else_expr);
	}
}

void Expr_Directive::destroy()
{
	for (int i = 0; i < arguments.size; i++) {
		arguments[i]->destroy();
		free(arguments[i]);
	}
	arguments.dealloc();
}

void Expr_Field::destroy()
{
	left->destroy();
	free(left);
}

void Expr_Loop::destroy()
{
	body->destroy();
	free(body);
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
	left->destroy();
	free(left);
	right->destroy();
	free(right);
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
	case STMT_RETURN:
		_return.destroy();
		break;
	case STMT_EXPR:
		expr->destroy();
		free(expr);
		break;
	case STMT_BREAK:
		_break->destroy();
		free(_break);
	}
}
