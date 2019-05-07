#define TAIL_CALL_OPTIMIZATION false

#include "includes.cc"
#include "allocator.cc"
#include "list.cc"
#include "global_alloc.cc"
#include "map.cc"
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
#include "environment.cc"
#include "bytecode.cc"
#include "value-def.cc"
#include "blocks.cc"
#include "builtins.cc"
#include "compiler.cc"
#include "vm.cc"

#define OUTPUT_BYTECODE false
#define DEBUG_OUTPUT false

const char * load_and_compile_file(Blocks * blocks, const char * filename)
{
	const char * source = load_string_from_file(filename);
	if (!source) {
		return NULL;
	}
	Lexer lexer;
	lexer.init(source);
	Parser parser;
	parser.init(&lexer);

	Compiler compiler;
	compiler.init(blocks);
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

	// Because file scopes are called just like functions, they need
	// to leave something behind on the stack.
	compiler.push(BC::create(BC_LOAD_CONST, Value::nothing(), -1));
	
	compiler.finalize();
	compiler.destroy();

	return source;
}

void work_from_source(const char * path)
{
	/*
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
		}*/

	Blocks blocks;
	blocks.init();
	const char * source = load_and_compile_file(&blocks, path);
	if (!source) {
		fatal("File '%s' does not exist!", path);
	}

	#if OUTPUT_BYTECODE
	for (int i = 0; i < blocks.blocks.size; i++) {
		auto block = blocks.blocks[i];
		auto size = blocks.sizes[i];
		printf("Block %d:\n", i);
		for (int j = 0; j < size; j++) {
			char * s = block[j].to_string();
			printf("%02d %s\n", j, s);
			free(s);
		}
	}
	#endif
	
	// Finally, just run our bytecode through the VM, starting with
	// `block_reference` 0
	VM vm;
	vm.init(&blocks, 0);
	while (!vm.halted()) {
		vm.step();
		#if DEBUG_OUTPUT
		vm.print_debug_info();
		#endif
	}
	#if DEBUG_OUTPUT
	vm.print_debug_info();
	#endif
	
	GC::unmark_all();
	vm.mark_reachable();
	GC::free_unmarked();
	
	blocks.destroy();
	vm.destroy();
}

int main(int argc, char ** argv)
{
	if (argc != 2) {
		fatal("Provide one source file");
	}

	Global_Alloc::init();
	Intern::init();
	GC::init();
	Builtins::init();
	Assoc_Allocator::init();
	
	work_from_source(argv[1]);

	Assoc_Allocator::destroy();
	Builtins::destroy();
	GC::destroy();
	Intern::destroy();
	Global_Alloc::destroy();
	
	return 0;
}
