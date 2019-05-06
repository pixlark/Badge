struct Parser {
	Lexer * lexer;
	Token peek;
	void init(Lexer * lexer)
	{
		this->lexer = lexer;
		advance();
	}
	Stmt * create_stmt(Stmt_Kind kind);
	Expr * create_expr(Expr_Kind kind);
	bool is(Token_Kind type);
	bool at_end();
	Token next();
	Token expect(Token_Kind type);
	Token weak_expect(Token_Kind type);
	void advance();
	bool match(Token_Kind type);

	// char overloads
	bool is(char c)           { return is((Token_Kind) c); }
	Token expect(char c)      { return expect((Token_Kind) c); }
	Token weak_expect(char c) { return weak_expect((Token_Kind) c); }
	bool match(char c)        { return match((Token_Kind) c); }

	Expr * parse_atom();
	Expr * parse_field_access();
	Expr * parse_function_call();
	Expr * parse_unary();
	Expr * parse_multiply_divide();
	Expr * parse_add_subtract();
	Expr * parse_comparisons();
	Expr * parse_equal_not_equal();
	Expr * parse_and();
	Expr * parse_or();
	List<Symbol> parse_symbol_list(Token_Kind open, Token_Kind close);
	Expr * parse_lambda();
	Expr * parse_loop();
	Expr * parse_if();
	Expr * parse_scope();
	Expr * parse_directive();
	Expr * parse_expr();
	
	Stmt * parse_let();
	Stmt * parse_set();
	Stmt * parse_return();
	Stmt * parse_break();
	Stmt * parse_stmt();
};

// Recursive descent chain

Expr * Parser::parse_atom()
{
	if (match('(')) {
		auto expr = parse_expr();
		expect(')');
		return expr;
	}
	switch (peek.kind) {
	case TOKEN_NOTHING: {
		auto atom = create_expr(EXPR_NOTHING);
		advance();
		return atom;
	} break;
	case TOKEN_INTEGER_LITERAL: {
		auto atom = create_expr(EXPR_INTEGER);
		atom->integer = peek.values.integer;
		advance();
		return atom;
	} break;
	case TOKEN_STRING_LITERAL: {
		auto atom = create_expr(EXPR_STRING);
		atom->string = peek.values.string;
		advance();
		return atom;
	} break;
	case TOKEN_SYMBOL: {
		auto atom = create_expr(EXPR_VARIABLE);
		atom->variable = peek.values.symbol;
		advance();
		return atom;
	} break;
	case TOKEN_THIS: {
		auto atom = create_expr(EXPR_THIS);
		advance();
		return atom;
	} break;
	default:
		fatal_assoc(peek.assoc, "Expected <int>, <symbol>; got %s", peek.to_string());
	}
	return NULL; // @linter
}

Expr * Parser::parse_field_access()
{
	auto left = parse_atom();
	while (match('\'')) {
		auto expr = create_expr(EXPR_FIELD);
		expr->field.left = left;
		weak_expect(TOKEN_SYMBOL);
		expr->field.right = peek.values.symbol;
		advance();
		left = expr;
	}
	return left;
}

Expr * Parser::parse_function_call()
{
	auto left = parse_field_access();
	while (match('(')) {
		auto expr = create_expr(EXPR_FUNCALL);
		expr->funcall.func = left;
		expr->funcall.args.alloc();
		while (true) {
			if (match(')')) {
				break;
			}
			expr->funcall.args.push(parse_expr());
			if (!match(',')) {
				expect(')');
				break;
			}
		}
		left = expr;
	}
	return left;
}

Expr * Parser::parse_unary()
{
	if (is('-') || is(TOKEN_NOT)) {
		Operator op;
		if (match('-')) {
			op = OP_NEGATE;
		} else if (match(TOKEN_NOT)) {
			op = OP_NOT;
		} else assert(false);
		auto expr = create_expr(EXPR_UNARY);
		expr->unary.op = op;
		expr->unary.expr = parse_unary();
		return expr;
	}
	return parse_function_call();
}

Expr * Parser::parse_multiply_divide()
{
	auto left = parse_unary();
	while (is('*') || is('/')) {
		Operator op;
		if (match('*')) {
			op = OP_MULTIPLY;
		} else if (match('/')) {
			op = OP_DIVIDE;
		} else assert(false);
		auto expr = create_expr(EXPR_BINARY);
		expr->binary.left = left;
		expr->binary.op = op;
		expr->binary.right = parse_unary();
		left = expr;
	}
	return left;
}

Expr * Parser::parse_add_subtract()
{
	auto left = parse_multiply_divide();
	while (is('+') || is('-')) {
		Operator op;
		if (match('+')) {
			op = OP_ADD;
		} else if (match('-')) {
			op = OP_SUBTRACT;
		} else assert(false);
		auto expr = create_expr(EXPR_BINARY);
		expr->binary.left = left;
		expr->binary.op = op;
		expr->binary.right = parse_multiply_divide();
		left = expr;
	}
	return left;
}

Expr * Parser::parse_comparisons()
{
	auto left = parse_add_subtract();
	while (is('<') || is('>') || is(TOKEN_LTE) || is(TOKEN_GTE)) {
		Operator op;
		if (match('<')) {
			op = OP_LESS_THAN;
		} else if (match('>')) {
			op = OP_GREATER_THAN;
		} else if (match(TOKEN_LTE)) {
			op = OP_LESS_THAN_OR_EQUAL_TO;
		} else if (match(TOKEN_GTE)) {
			op = OP_GREATER_THAN_OR_EQUAL_TO;
		} else assert(false);
		auto expr = create_expr(EXPR_BINARY);
		expr->binary.left = left;
		expr->binary.op = op;
		expr->binary.right = parse_add_subtract();
		left = expr;
	}
	return left;
}

Expr * Parser::parse_equal_not_equal()
{
	auto left = parse_comparisons();
	while (is(TOKEN_EQUAL) || is(TOKEN_NOT_EQUAL)) {
		Operator op;
		if (match(TOKEN_EQUAL)) {
			op = OP_EQUAL;
		} else if (match(TOKEN_NOT_EQUAL)) {
			op = OP_NOT_EQUAL;
		} else assert(false);
		auto expr = create_expr(EXPR_BINARY);
		expr->binary.left = left;
		expr->binary.op = op;
		expr->binary.right = parse_comparisons();
		left = expr;
	}
	return left;
}

Expr * Parser::parse_and()
{
	auto left = parse_equal_not_equal();
	while (match(TOKEN_AND)) {
		auto expr = create_expr(EXPR_BINARY);
		expr->binary.left = left;
		expr->binary.op = OP_AND;
		expr->binary.right = parse_equal_not_equal();
		left = expr;
	}
	return left;
}

Expr * Parser::parse_or()
{
	auto left = parse_and();
	while (match(TOKEN_OR)) {
		auto expr = create_expr(EXPR_BINARY);
		expr->binary.left = left;
		expr->binary.op = OP_OR;
		expr->binary.right = parse_and();
		left = expr;
	}
	return left;
}

List<Symbol> Parser::parse_symbol_list(Token_Kind open, Token_Kind close)
{
	expect(open);
	List<Symbol> list;
	list.alloc();
	while (true) {
		if (match(close)) {
			break;
		}
		weak_expect(TOKEN_SYMBOL);
		list.push(peek.values.symbol);
		advance();
		if (!match(',')) {
			expect(close);
			break;
		}
	}
	return list;
}

Expr * Parser::parse_lambda()
{
	if (match(TOKEN_LAMBDA)) {
		auto lambda = create_expr(EXPR_LAMBDA);
		lambda->lambda.parameters = parse_symbol_list((Token_Kind) '(',
													  (Token_Kind) ')');
		lambda->lambda.body = parse_expr();
		return lambda;
	} else {
		return parse_or();
	}
}

Expr * Parser::parse_loop()
{
	if (match(TOKEN_LOOP)) {
		// Check for `for` expression
		auto loop = create_expr(EXPR_LOOP);
		if (match(TOKEN_FOR)) {
			loop->loop.for_expr = parse_expr();
		} else {
			loop->loop.for_expr = NULL;
		}
		loop->loop.body = parse_expr();
		return loop;
	} else {
		return parse_lambda();
	}
}

Expr * Parser::parse_if()
{
	if (match(TOKEN_IF)) {
		auto expr = create_expr(EXPR_IF);
		expr->if_expr.conditions.alloc();
		expr->if_expr.expressions.alloc();
		do {
			expr->if_expr.conditions.push(parse_expr());
			expect(TOKEN_THEN);
			expr->if_expr.expressions.push(parse_expr());
		} while (match(TOKEN_ELIF));
		if (match(TOKEN_ELSE)) {
			expr->if_expr.else_expr = parse_expr();
		} else {
			expr->if_expr.else_expr = NULL;
		}
		return expr;
	} else {
		return parse_loop();
	}
}

Expr * Parser::parse_directive()
{
	if (match('@')) {
		auto expr = create_expr(EXPR_DIRECTIVE);
		expr->directive.name = expect(TOKEN_SYMBOL).values.symbol;
		expr->directive.arguments.alloc();
		expect('[');
		while (true) {
			if (match(']')) {
				break;
			}
			expr->directive.arguments.push(parse_expr());
			if (!match(',')) {
				expect(']');
				break;
			}
		}
		return expr;
	} else {
		return parse_if();
	}
}

Expr * Parser::parse_scope()
{
	if (match('{')) {
		auto scope = create_expr(EXPR_SCOPE);
		scope->scope.body.alloc();
		scope->scope.terminator = NULL;
		while (!match('}')) {
			auto stmt = parse_stmt();
			if (stmt->kind == STMT_EXPR && match('}')) {
				scope->scope.terminator = stmt->expr;
				free(stmt);
				break;
			}
			expect('.');
			scope->scope.body.push(stmt);
		}
		return scope;
	} else {
		return parse_directive();
	}
}

Expr * Parser::parse_expr()
{
	return parse_scope();
}

Stmt * Parser::parse_let()
{
	Stmt * stmt = create_stmt(STMT_LET);
	advance();
	
	weak_expect(TOKEN_SYMBOL);
	stmt->let.left = peek.values.symbol;
	advance();

	expect('=');

	stmt->let.right = parse_expr();
	
	return stmt;
}

Stmt * Parser::parse_set()
{
	Stmt * stmt = create_stmt(STMT_SET);
	advance();
	
	//stmt->set.left = peek.values.symbol;
	stmt->set.left = parse_expr();
	//advance();

	expect('=');

	stmt->set.right = parse_expr();
	
	return stmt;	
}

Stmt * Parser::parse_return()
{
	Stmt * stmt = create_stmt(STMT_RETURN);
	advance();
	stmt->_return.expr = parse_expr();
	return stmt;	
}

Stmt * Parser::parse_break()
{
	expect(TOKEN_BREAK);
	auto stmt = create_stmt(STMT_BREAK);
	stmt->_break = parse_expr();
	return stmt;
}

Stmt * Parser::parse_stmt()
{
	Stmt * stmt;
	if (is(TOKEN_LET)) {
		stmt = parse_let();
	} else if (is(TOKEN_SET)) {
		stmt = parse_set();
	} else if (is(TOKEN_RETURN)) {
		stmt = parse_return();
	} else if (is(TOKEN_BREAK)) {
		stmt = parse_break();
	} else {
		stmt = create_stmt(STMT_EXPR);
		stmt->expr = parse_expr();
	}
	return stmt;
}

// Parser utility

Stmt * Parser::create_stmt(Stmt_Kind kind)
{
	auto stmt = Stmt::with_kind(kind, peek.assoc);
	return stmt;
}

Expr * Parser::create_expr(Expr_Kind kind)
{
	auto expr = Expr::with_kind(kind, peek.assoc);
	return expr;
}

bool Parser::is(Token_Kind type)
{
	return peek.kind == type;
}

bool Parser::at_end()
{
	return peek.kind == TOKEN_EOF;
}

Token Parser::next()
{
	Token t = peek;
	advance();
	return t;
}

Token Parser::expect(Token_Kind type)
{
	if (!is(type)) {
		fatal_assoc(peek.assoc,
					"Expected %s, got %s",
					Token::type_to_string(type),
					peek.to_string());
	}
	return next();
}

Token Parser::weak_expect(Token_Kind type)
{
	if (!is(type)) {
		fatal_assoc(peek.assoc,
					"Expected %s, got %s",
					Token::type_to_string(type),
					peek.to_string());
	}
	return peek;
}

void Parser::advance()
{
	this->peek = lexer->next_token();
}

bool Parser::match(Token_Kind type)
{
	if (is(type)) {
		advance();
		return true;
	}
	return false;
}
