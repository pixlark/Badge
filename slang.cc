#include "list.h"
#include "includes.cc"
#include "string-builder.cc"
#include "intern.cc"
#include "utility.cc"
#include "error.cc"
#include "lexer.cc"
#include "ast.cc"
#include "parser.cc"
#include "gc.cc"
#include "gc-structs.cc"
#include "value-decl.cc"
#include "bytecode.cc"
#include "value-def.cc"
#include "compiler.cc"
#include "vm.cc"

#define DEBUG false

void work_from_source(const char * path)
{
	const char * source = load_string_from_file(path);

	if (!source) {
		fatal("File does not exist");
	}

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
	}

	vm.destroy();
	free((char*) source);
}

void repl()
{
	VM vm;
	vm.init();
	
	while (true) {
		printf("-> ");
		char buf[512];
		fgets(buf, 512, stdin);

		Lexer lexer;
		lexer.init(buf);
		Parser parser;
		parser.init(&lexer);
		
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
		}
	}
	
	vm.destroy();
}

int main(int argc, char ** argv)
{
	bool is_repl;

	if (argc == 1) {
		is_repl = true;
	} else if (argc == 2) {
		is_repl = false;
	} else {
		fatal("Cannot execute multiple scripts");
	}

	Intern::init();
	GC::init();
	
	if (is_repl) {
		repl();
	} else {
		work_from_source(argv[1]);
	}

	GC::destroy();
	Intern::destroy();
	
	return 0;
}
