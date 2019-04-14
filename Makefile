make:
	g++ -Wall -g -O0 main.cc -o slang -Wno-sign-compare

clang:
	clang++ -Wall --std=c++11 -g -O0 main.cc -o slang

tidy:
	clang-tidy -extra-arg-before='--std=c++11' main.cc
