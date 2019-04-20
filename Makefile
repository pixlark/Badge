make:
	g++ -Wall -g -O0 src/main.cc -o badge -Wno-sign-compare

clang:
	clang++ -Wall --std=c++11 -g -O0 src/main.cc -o badge

lint:
	clang-tidy src/main.cc -- --std=c++11
