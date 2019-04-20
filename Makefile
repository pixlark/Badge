UNITY_FILE=src/main.cc
OUTPUT_NAME=badge

make:
	g++ -Wall -g -O0 $(UNITY_FILE) -o $(OUTPUT_NAME) -Wno-sign-compare

release:
	clang++ -Wall --std=c++11 -O3 $(UNITY_FILE) -o $(OUTPUT_NAME)

clang:
	clang++ -Wall --std=c++11 -g -O0 $(UNITY_FILE) -o $(OUTPUT_NAME)

lint:
	clang-tidy $(UNITY_FILE) -- --std=c++11
