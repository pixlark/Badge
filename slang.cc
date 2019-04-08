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

int main(int argc, char ** argv)
{
	if (argc != 2) {
		fatal("Provide one file to execute");
	}

	const char * source = load_string_from_file(argv[1]);

	if (!source) {
		fatal("File does not exist");
	}
	
	Intern::init();

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
	free((char*) source);
	
	return 0;
}
