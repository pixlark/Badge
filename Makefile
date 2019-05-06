UNITY_FILE=src/main.cc
OUTPUT_NAME=badge

RESET="\\e[0m"
BOLD="\\e[1m"

make:
	g++ -Wall -g -O0 $(UNITY_FILE) -o $(OUTPUT_NAME) -Wno-sign-compare

release:
	clang++ -Wall --std=c++11 -DRELEASE -O3 $(UNITY_FILE) -o $(OUTPUT_NAME)

clang:
	clang++ -Wall --std=c++11 -g -O0 $(UNITY_FILE) -o $(OUTPUT_NAME)

lint:
	clang-tidy $(UNITY_FILE) -- --std=c++11

full-test:
	@printf "\e[1mBASIC TESTS\e[0m:\n\n" && \
	lesen tests && \
	printf "\n\e[1mLEAK CHECKS\e[0m:\n\n" && \
	lesen --leak-check tests

