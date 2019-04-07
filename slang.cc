#include "list.h"
#include "includes.cc"
#include "string-builder.cc"
#include "intern.cc"
#include "utility.cc"
#include "error.cc"
#include "lexer.cc"
#include "ast.cc"
#include "parser.cc"
#include "value.cc"
#include "bytecode.cc"
#include "compiler.cc"
#include "vm.cc"

int main()
{
	Intern::init();

	const char * source =
		"let x = 12;"
		"set x = 11;";

	Lexer lexer;
	lexer.init(source);
	Parser parser;
	parser.init(&lexer);
	
	VM vm;
	vm.init();
	while (!parser.is(TOKEN_EOF)) {
		auto stmt = parser.parse_stmt();
		{
			char * s = stmt->to_string();
			printf("%s\n\n", s);
			free(s);
		}

		Compiler compiler;
		compiler.init();
		compiler.compile_stmt(stmt);
	
		vm.prime(compiler.bytecode);
		while (!vm.halted()) {
			vm.step();
			vm.print_debug_info();
		}	
	}
	
	return 0;
}
