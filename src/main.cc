#define TAIL_CALL_OPTIMIZATION true

#include "includes.cc"
#include "defer.cc"
#include "allocator.cc"
#include "list.cc"
#include "global_alloc.cc"
#include "map.cc"
#include "string-builder.cc"
#include "intern.cc"
#include "utility.cc"
#include "error.cc"
#include "files.cc"
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
		defer {
			stmt->destroy();
			free(stmt);
		};
		
		// Top-level expects terminators for every statement
		parser.expect('.');

		// Here we generate bytecode from our abstract syntax tree
		// (one statement's worth)
		compiler.compile_stmt(stmt);
	}

	// Because file scopes are called just like functions, they need
	// to leave something behind on the stack.
	compiler.push(BC::create(BC_LOAD_CONST, Value::nothing(), -1));
	
	compiler.finalize();
	compiler.destroy();

	return source;
}

void run(Blocks * blocks, )
{
	
}

void repl()
{
	Blocks blocks;
	blocks.init();

	auto export_scope = Environment::alloc();
	
}

void work_from_source(const char * path)
{
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
	
	auto export_scope = Environment::alloc();
	
	// Finally, just run our bytecode through the VM, starting with
	// `block_reference` 0
	List<VM> vm_stack;
	vm_stack.alloc();

	{
		VM vm;
		vm.init(&blocks, export_scope, 0);
		vm_stack.push(vm);
	}

	while (vm_stack.size > 0) {
		auto vm = &vm_stack[vm_stack.size - 1];
		auto response = vm->step();
		switch (response) {
		case VM_OK:
			break;
		case VM_HALTED: {
			vm->destroy();
			vm_stack.pop();
		} break;
		case VM_SWITCH: {
			VM new_vm;
			new_vm.init(&blocks, export_scope, vm->block_reference_to_push);
			vm_stack.push(new_vm);
		} break;
		default:
			assert(false);
		}

		#if DEBUG_OUTPUT
		vm->print_debug_info();
		#endif
		
		// This should ALWAYS run before the loop breaks, that way
		// anything that needs to get cleaned up, will be.
		#if COLLECTION
		do {
			#if RELEASE
			if (!GC::should_collect()) {
				break;
			}
			GC::reset_heuristics();
			#endif
			GC::unmark_all();
			for (int i = 0; i < vm_stack.size; i++) {
				vm_stack[i].mark_reachable();
			}
			GC::free_unmarked();
		} while(0);
		#endif
	}
	
	assert(vm_stack.size == 0);
	vm_stack.dealloc();
	
	blocks.destroy();
}

int main(int argc, char ** argv)
{
	bool repl = argc == 1;
	
	if (!repl && argc != 2) {
		fatal("Provide one source file");
	}

	Files::init(argv[1]);
	Global_Alloc::init();
	Intern::init();
	GC::init();
	Builtins::init();
	Assoc_Allocator::init();

	if (repl) {
		repl();
	} else {
		work_from_source(argv[1]);
	}

	Assoc_Allocator::destroy();
	Builtins::destroy();
	GC::destroy();
	Intern::destroy();
	Global_Alloc::destroy();
	Files::destroy();
	
	return 0;
}
