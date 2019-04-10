make:
	g++ -g -O0 slang.cc -o slang -Wswitch

clang:
	clang++ --std=c++11 -g -O0 slang.cc -o slang

tidy:
	clang-tidy -extra-arg-before='--std=c++11' slang.cc
