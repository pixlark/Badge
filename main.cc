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
#include "value-decl.cc"
#include "bytecode.cc"
#include "value-def.cc"
#include "blocks.cc"
#include "compiler.cc"
#include "vm.cc"

#define DEBUG false

void work_from_source(const char * path)
{
	const char * source;
	
	if (strcmp(path, "-") == 0) {
		// Read from stdin
		String_Builder builder;
		const int chunk_size = 64;
		char buf[chunk_size];
		
		size_t read;
		while ((read = fread(buf, sizeof(char), chunk_size, stdin)) == chunk_size) {
			builder.append_size(buf, chunk_size);
		}
		if (ferror(stdin)) {
			fatal("Error reading from stdin");
		}
		assert(read <= chunk_size);
		builder.append_size(buf, read);
		source = builder.final_string();
	} else {
		// Read from file
		source = load_string_from_file(path);
		if (!source) {
			fatal("File does not exist");
		}
	}

	Lexer lexer;
	lexer.init(source);
	Parser parser;
	parser.init(&lexer);

	// `Blocks` stores all compiled bytecode in reverse dependent
	// order. This is for freeing/serialization, as well as
	// references from the VM.
	Blocks blocks;
	blocks.init();

	// Our main compiler; will have `block_reference` of 0
	Compiler compiler;
	compiler.init(&blocks);

	while (!parser.is(TOKEN_EOF)) {
		// The parser feeds from the lexer and returns one
		// statement's worth of abstract syntax tree
		auto stmt = parser.parse_stmt();
		// Top-level expects terminators for every statement
		parser.expect('.');

		// Here we generate bytecode from our abstract syntax tree
		// (one statement's worth)
		compiler.compile_stmt(stmt);

		// Abstract syntax tree gets freed
		stmt->destroy();
		free(stmt);
	}

	// Finalize our zeroth block and clean up compiler
	compiler.finalize();
	compiler.destroy();
	
	// Finally, just run our bytecode through the VM, starting with
	// `block_reference` 0
	VM vm;
	vm.init(&blocks, 0);
	while (!vm.halted()) {
		vm.step();
		#if DEBUG
		vm.print_debug_info();
		#endif
	}
	#if DEBUG
	vm.print_debug_info();
	#endif
	
	blocks.destroy();
	vm.destroy();
	free((char*) source);
}

int main(int argc, char ** argv)
{
	if (argc != 2) {
		fatal("Provide one source file");
	}

	Intern::init();
	GC::init();
	
	work_from_source(argv[1]);

	GC::destroy();
	Intern::destroy();
	
	return 0;
}
