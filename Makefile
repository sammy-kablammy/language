C_COMPILER=gcc -std=c99 -Werror -Wall -Wextra -Wpedantic -g
OUT=compiler

main: main.c
	$(C_COMPILER) -o $(OUT) main.c

clean:
	rm $(OUT)
