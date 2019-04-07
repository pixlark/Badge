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
#include "vm.cc"

int main()
{
	Intern::init();

	/*
	const char * source = "let x = 12;";
	Lexer lexer;
	lexer.init(source);

	Parser parser;
	parser.init(&lexer);
	
	auto stmt = parser.parse_stmt();
	printf("%s\n", stmt->to_string());*/

	List<BC> bytecode;
	bytecode.alloc();
	bytecode.push(BC::create(BC_LOAD_CONST, Value::raise(2)));
	bytecode.push(BC::create(BC_LOAD_CONST, Value::raise(Intern::intern("x"))));
	bytecode.push(BC::create(BC_CREATE_BINDING));

	bytecode.push(BC::create(BC_LOAD_CONST, Value::raise(3)));
	bytecode.push(BC::create(BC_LOAD_CONST, Value::raise(Intern::intern("x"))));
	bytecode.push(BC::create(BC_UPDATE_BINDING));

	VM vm;
	vm.init();
	vm.prime(bytecode);
	while (!vm.halted()) {
		vm.step();
		vm.print_debug_info();
	}
	
	return 0;
}
