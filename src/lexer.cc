enum Token_Kind {
	TOKEN_EOF = 256,

	// Double-chars
	TOKEN_EQUAL,
	TOKEN_NOT_EQUAL,
	TOKEN_GTE,
	TOKEN_LTE,
	
	// Reserved words
	TOKEN_LET,
	TOKEN_SET,
	TOKEN_LAMBDA,
	TOKEN_RETURN,
	TOKEN_NOTHING,
	TOKEN_OR,
	TOKEN_AND,
	TOKEN_NOT,
	TOKEN_IF,
	TOKEN_THEN,
	TOKEN_ELIF,
	TOKEN_ELSE,
	TOKEN_THIS,
	TOKEN_LOOP,
	TOKEN_BREAK,
	TOKEN_WHILE,
	
	TOKEN_SYMBOL,
	TOKEN_INTEGER_LITERAL,
	TOKEN_STRING_LITERAL,
};

#define RESERVED_WORDS_BEGIN (TOKEN_LET)
#define RESERVED_WORDS_END   (TOKEN_SYMBOL)
#define RESERVED_WORDS_COUNT (RESERVED_WORDS_END - RESERVED_WORDS_BEGIN)

static const char * reserved_words[RESERVED_WORDS_COUNT] = {
	"let", "set", "lambda", "return",
	"nothing", "or", "and", "not", "if", "then", "elif",
	"else", "this", "loop", "break", "while",
};

struct Token {
	Token_Kind kind;
	Assoc_Ptr assoc;
	union {
		int integer;
		Symbol symbol;
		Symbol string; // String literals are technically symbols
	} values;
	static Token with_kind(Token_Kind type)
	{
		Token token;
		token.kind = type;
		return token;
	}
	static char * type_to_string(Token_Kind type);
	char * to_string();
};

char * Token::type_to_string(Token_Kind kind)
{
	if (kind < 256) {
		return Token::with_kind(kind).to_string();
	}
	
	if (kind >= RESERVED_WORDS_BEGIN && kind < RESERVED_WORDS_END) {
		return strdup(reserved_words[kind - RESERVED_WORDS_BEGIN]);
	}
	
	switch (kind) {
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
	if (kind >= 0 && kind < 256) {
		char buf[2];
		buf[0] = kind;
		buf[1] = '\0';
		return strdup(buf);
	}
	switch (kind) {
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
		return Token::type_to_string(kind);
	}
}

struct Lexer {
	const char * source;
	size_t source_length;
	size_t cursor;
	size_t line;
	Token create_token(Token_Kind kind, size_t size);
	void init(const char * source);
	char next();
	char peek();
	void advance();
	Token next_token();
	Token_Kind read_double_token(char left, char right, Token_Kind double_type);
};

Token Lexer::create_token(Token_Kind kind, size_t size)
{
	auto token = Token::with_kind(kind);
	token.assoc = Assoc_Allocator::make(source, source_length, cursor - size, size, line);
	return token;
}

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
		Token token = create_token(TOKEN_EOF, 1);
		return token;
	}
	
	if (isspace(peek())) {
		advance();
		goto reset;
	}

	if (peek() == '%') {
		while (peek() != '\0' && peek() != '\n') {
			advance();
		}
		goto reset;
	}
	
	if (peek() == '"') {
		size_t start = cursor;
		advance();
		// TODO(pixlark): Buffer overflow
		char buf[512];
		size_t i = 0;
		while (peek() != '"') {
			buf[i++] = next();
		}
		buf[i++] = '\0';
		advance();
		
		Token token = create_token(TOKEN_STRING_LITERAL, cursor - start);
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
				return create_token((Token_Kind) (RESERVED_WORDS_BEGIN + i), strlen(buf));
				//return Token::with_kind((Token_Kind) (RESERVED_WORDS_BEGIN + i));
			}
		}
		
		Token token = create_token(TOKEN_SYMBOL, strlen(buf));
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
		Token token = create_token(TOKEN_INTEGER_LITERAL, strlen(buf));
		token.values.integer = strtol(buf, NULL, 10);
		return token;
	}
	
	switch (peek()) {
	case '+':
	case '-':
	case '*':
	case '/':
	case '.':
	case ',':
	case '(':
	case ')':
	case '{':
	case '}':
	case '@':
	case '\'':
		return create_token((Token_Kind) next(), 1);
		//return Token::with_kind((Token_Kind) next());
	case '=':
		return create_token(read_double_token('=', '=', TOKEN_EQUAL), 1);
	case '!':
		return create_token(read_double_token('!', '=', TOKEN_NOT_EQUAL), 1);
	case '>':
		return create_token(read_double_token('>', '=', TOKEN_GTE), 1);
	case '<':
		return create_token(read_double_token('<', '=', TOKEN_LTE), 1);
	case ']':
		return create_token((Token_Kind) next(), 1);
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
		} else {
			return create_token((Token_Kind) '[', 1);
			//return Token::with_kind((Token_Kind) '[');
		}
	default:
		fatal_assoc(Assoc_Allocator::make(source, source_length, cursor, 1, line),
					"Line %d\nMisplaced character %c (%d)", line, peek(), peek());
		/*
	case ';':
		return Token::with_kind(read_double_token(';', ';', TOKEN_DOUBLE_SEMICOLON));*/
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
