struct Parser {
	Lexer * lexer;
	Token peek;
	void init(Lexer * lexer)
	{
		this->lexer = lexer;
		advance();
	}
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
	List<Symbol> parse_symbol_list();
	Expr * parse_lambda();
	Expr * parse_expr();
	
	Stmt * parse_let();
	Stmt * parse_set();
	Stmt * parse_print();
	Stmt * parse_stmt();
};

// Recursive descent chain

Expr * Parser::parse_atom()
{
	switch (peek.type) {
	case TOKEN_INTEGER_LITERAL: {
		Expr * atom = Expr::with_kind(EXPR_INTEGER);
		atom->integer = peek.values.integer;
		advance();
		return atom;
	} break;
	case TOKEN_SYMBOL: {
		Expr * atom = Expr::with_kind(EXPR_VARIABLE);
		atom->variable = peek.values.symbol;
		advance();
		return atom;
	} break;
	default:
		fatal("Expected <int>, <symbol>; got %s", peek.to_string());
	}
}

List<Symbol> Parser::parse_symbol_list()
{
	expect('(');
	List<Symbol> list;
	list.alloc();
	while (true) {
		if (match(')')) {
			break;
		}
		weak_expect(TOKEN_SYMBOL);
		list.push(peek.values.symbol);
		advance();
		if (!match(',')) {
			expect(')');
			break;
		}
	}
	return list;
}

Expr * Parser::parse_lambda()
{
	if (match(TOKEN_LAMBDA)) {
		auto lambda = Expr::with_kind(EXPR_LAMBDA);
		lambda->lambda.parameters = parse_symbol_list();
		expect('{');
		lambda->lambda.body.alloc();
		while (!match('}')) {
			lambda->lambda.body.push(parse_stmt());
		}
		return lambda;
	} else {
		return parse_atom();
	}
}

Expr * Parser::parse_expr()
{
	return parse_lambda();
}

Stmt * Parser::parse_let()
{
	Stmt * stmt = Stmt::with_kind(STMT_LET);
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
	Stmt * stmt = Stmt::with_kind(STMT_SET);
	advance();
	
	weak_expect(TOKEN_SYMBOL);
	stmt->set.left = peek.values.symbol;
	advance();

	expect('=');

	stmt->set.right = parse_expr();
	
	return stmt;	
}

Stmt * Parser::parse_print()
{
	Stmt * stmt = Stmt::with_kind(STMT_PRINT);
	advance();
	stmt->print.expr = parse_expr();
	return stmt;
}

Stmt * Parser::parse_stmt()
{
	Stmt * stmt;
	if (is(TOKEN_LET)) {
		stmt = parse_let();
	} else if (is(TOKEN_SET)) {
		stmt = parse_set();
	} else if (is(TOKEN_PRINT)) {
		stmt = parse_print();
	} else {
		fatal("Expected let, set; got %s", peek.to_string());
	}
	expect(';');
	return stmt;
}

// Parser utility

bool Parser::is(Token_Kind type)
{
	return peek.type == type;
}

bool Parser::at_end()
{
	return peek.type == TOKEN_EOF;
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
		fatal("Expected %s, got %s",
			  Token::type_to_string(type),
			  peek.to_string());
	}
	return next();
}

Token Parser::weak_expect(Token_Kind type)
{
	if (!is(type)) {
		fatal("Expected %s, got %s",
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
