// This project is a unity build / single compilation unit build.
#include "platform.c"
#include "lex.c"
#include "error_messages.c"
#include "parse.c"
#include "pretty_print.c"

void print_expression(expression_node e);
void print_leaves(infix_node* node);

void lex_error(const char *fmt, ...);

int main(int argc, char **argv) {
    if (argc < 2) {
        panic("Usage: '%s <file>'", argv[0]);
    }

    token tok = { .kind = TOKEN_PLUS, };
    compiler_printf("Token '%T' Yeah %d tokens are cool\n", tok, 100);
    string str = { .buf = "Hello", .length = sizeof("Hello") - 1 };
    compiler_printf("String '%S' and some percents maybe 100%%%%yup\n", str);
    // compiler_printf("Bruh %");

    char buf[10000] = { 0 };;
    compiler_sprintf(buf, "String '%S' and tok %T\n", str, tok);
    printf("%s\n", buf);

    char buf2[10000] = { 0 };
    int written = compiler_sprintf(buf2, "%d", 1234);
    printf("written=%d\n", written);

    fflush(stdout);
    fflush(stderr);
    return 0;

    // handle compiler testing arguments (intended for internal use only)
    bool pretty_print_and_exit = false;
    int arg_idx = 1;
    while (arg_idx < argc - 1) {
        char *arg = argv[arg_idx];
        // Not all valid ASTs pass type checking. Allow parser test cases to
        // continue working even as later compiler phases are implemented
        if (cstring_equals_cstring(arg, "--pretty-print-and-exit")) {
            pretty_print_and_exit = true;
        }
        arg_idx += 1;
    }
    string contents = read_entire_file(argv[arg_idx]);

    token_array tokens = lex_entire_file(contents);

	statement_node *first_statement = parse_entire_file(tokens);

    if (pretty_print_and_exit) {
        pretty_print_entire_file(first_statement);
        return 0;
    }

	return 0;
}
