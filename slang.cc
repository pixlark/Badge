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

#define DEBUG false

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
			#if DEBUG
			printf("%s\n\n", s);
			#endif
			free(s);
		}

		Compiler compiler;
		compiler.init();
		compiler.compile_stmt(stmt);

		stmt->destroy();
		free(stmt);
		
		vm.prime(compiler.bytecode);
		while (!vm.halted()) {
			vm.step();
			#if DEBUG
			vm.print_debug_info();
			#endif
		}

		compiler.destroy();
	}

	vm.destroy();
	
	return 0;
}
