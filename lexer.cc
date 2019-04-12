enum Token_Kind {
	TOKEN_EOF = 256,
	
	// Reserved words
	TOKEN_LET,
	TOKEN_SET,
	TOKEN_PRINT,
	TOKEN_LAMBDA,
	TOKEN_RETURN,
	
	TOKEN_SYMBOL,
	TOKEN_INTEGER_LITERAL,
	TOKEN_STRING_LITERAL,
};

#define RESERVED_WORDS_BEGIN (TOKEN_LET)
#define RESERVED_WORDS_END   (TOKEN_SYMBOL)
#define RESERVED_WORDS_COUNT (RESERVED_WORDS_END - RESERVED_WORDS_BEGIN)

static const char * reserved_words[RESERVED_WORDS_COUNT] = {
	"let", "set", "print", "lambda", "return", 
};

struct Token {
	Token_Kind type;
	union {
		int integer;
		Symbol symbol;
		Symbol string; // String literals are technically symbols
	} values;
	static Token eof()
	{
		Token token;
		token.type = TOKEN_EOF;
		return token;
	}
	static Token with_type(Token_Kind type)
	{
		Token token;
		token.type = type;
		return token;
	}
	static char * type_to_string(Token_Kind type);
	char * to_string();
};

char * Token::type_to_string(Token_Kind type)
{
	if (type < 256) {
		return Token::with_type(type).to_string();
	}
	
	if (type >= RESERVED_WORDS_BEGIN && type < RESERVED_WORDS_END) {
		return strdup(reserved_words[type - RESERVED_WORDS_BEGIN]);
	}
	
	switch (type) {
	case TOKEN_EOF:
		return strdup("EOF");
	case TOKEN_SYMBOL:
		return strdup("<symbol>");
	case TOKEN_INTEGER_LITERAL:
		return strdup("<integer>");
	case TOKEN_STRING_LITERAL:
		return strdup("<string>");
	default:
		fatal("Token::type_to_string() switch incomplete");
	}
	return NULL; // @linter
}

char * Token::to_string()
{
	if (type >= 0 && type < 256) {
		char buf[2];
		buf[0] = type;
		buf[1] = '\0';
		return strdup(buf);
	}
	switch (type) {
	case TOKEN_SYMBOL: {
		return strdup(values.symbol);
	}
	case TOKEN_INTEGER_LITERAL: {
		return itoa(values.integer);
	}
	case TOKEN_STRING_LITERAL: {
		String_Builder builder;
		builder.append("string \"");
		builder.append(values.string);
		builder.append("\"");
		return builder.final_string();
	}
	default:
		return Token::type_to_string(type);
	}
}

struct Lexer {
	const char * source;
	size_t source_length;
	size_t cursor;
	size_t line;
	void init(const char * source);
	char next();
	char peek();
	void advance();
	Token next_token();
	Token_Kind read_double_token(char left, char right, Token_Kind double_type);
};

void Lexer::init(const char * source)
{
	this->source = source;
	source_length = strlen(source);
	cursor = 0;
	line = 1;
}

char Lexer::next()
{
	char c = peek();
	advance();
	return c;
}

char Lexer::peek()
{
	if (cursor >= source_length) {
		return '\0';
	}
	return source[cursor];
}

void Lexer::advance()
{
	if (cursor >= source_length) {
		return;
	}
	if (source[cursor] == '\n') {
		line++;
	}
	cursor++;
}

Token Lexer::next_token()
{
 reset:
	if (peek() == '\0') {
		return Token::eof();
	}
	
	if (isspace(peek())) {
		advance();
		goto reset;
	}

	if (peek() == '%') {
		while (peek() != '\n') {
			advance();
		}
		goto reset;
	}
	
	if (peek() == '"') {
		advance();
		// TODO(pixlark): Buffer overflow
		char buf[512];
		size_t i = 0;
		while (peek() != '"') {
			buf[i++] = next();
		}
		buf[i++] = '\0';
		advance();
		
		Token token;
		token.type = TOKEN_STRING_LITERAL;
		token.values.string = Intern::intern(buf); // string literals are technically symbols
		return token;
	}
	
	if (isalpha(peek()) || peek() == '_') {
		// TODO(pixlark): Buffer overflow
		char buf[512];
		size_t i = 0;
		while (isalnum(peek()) || peek() == '_') {
			buf[i++] = next();
		}
		buf[i++] = '\0';

		for (int i = 0; i < RESERVED_WORDS_COUNT; i++) {
			if (strcmp(buf, reserved_words[i]) == 0) {
				return Token::with_type((Token_Kind) (RESERVED_WORDS_BEGIN + i));
			}
		}
		
		Token token;
		token.type = TOKEN_SYMBOL;
		token.values.symbol = Intern::intern(buf);
		return token;
	}

	if (isdigit(peek())) {
		// TODO(pixlark): Buffer overflow
		char buf[512];
		size_t i = 0;
		while (isdigit(peek())) {
			buf[i++] = next();
		}
		buf[i++] = '\0';
		Token token;
		token.type = TOKEN_INTEGER_LITERAL;
		token.values.integer = strtol(buf, NULL, 10);
		return token;
	}
	
	switch (peek()) {
	case ';':
	case '=':
	case ',':
	case '(':
	case ')':
	case '{':
	case '}':
		return Token::with_type((Token_Kind) next());
	case '[':
		advance();
		if (peek() == '-') {
			// Multi-line comment
			while (true) {
				advance();
				if (peek() == '\0') {
					fatal("Unterminated multi-line comment");
				}
				if (peek() == '-') {
					advance();
					if (peek() == ']') {
						advance();
						break;
					}
				}
			}
			goto reset;
		}
		/* FALLTHROUGH */
	default:
		fatal("Line %d\nMisplaced character %c (%d)", line, peek(), peek());
		/*
	case ';':
		return Token::with_type(read_double_token(';', ';', TOKEN_DOUBLE_SEMICOLON));*/
	}
	return (Token) {}; // @linter
}

Token_Kind Lexer::read_double_token(char left, char right, Token_Kind double_type)
{
	assert(peek() == left);
	advance();
	if (peek() == right) {
		advance();
		return double_type;
	} else {
		return (Token_Kind) left;
	}
}
