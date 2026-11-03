C_COMPILER=gcc -std=c99 -Werror -Wall -Wextra -Wpedantic -g
OUT=compiler

main: main.c
	$(C_COMPILER) -o $(OUT) main.c

test: main
	./parser_test_cases/test.sh
	@echo ""
	./typecheck_test_cases/test.sh

clean:
	rm $(OUT)
	rm ./parser_test_cases/*.got
	rm ./typecheck_test_cases/*.got
