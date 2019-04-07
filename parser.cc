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

	Expr * parse_atom();
	Expr * parse_expr();
	Stmt * parse_let();
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
	default:
		fatal("Expected <int>, got %s", peek.to_string());
	}
}

Expr * Parser::parse_expr()
{
	return parse_atom();
}

Stmt * Parser::parse_let()
{
	Stmt * stmt = Stmt::with_kind(STMT_LET);
	advance();
	
	weak_expect(TOKEN_SYMBOL);
	stmt->let.left = peek.values.symbol;
	advance();

	expect((Token_Kind) '=');

	stmt->let.right = parse_expr();
	
	return stmt;
}

Stmt * Parser::parse_stmt()
{
	Stmt * stmt;
	if (is(TOKEN_LET)) {
		stmt = parse_let();
	} else {
		fatal("Expected let, got %s", peek.to_string());
	}
	expect((Token_Kind) ';');
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
