// This project is a unity build / single compilation unit build.
#include "platform.c"
#include "array.c"
#include "lex.c"
#include "compiler_printf.c"
#include "parse.c"
#include "pretty_print.c"
#include "hash_table.c"
#include "typecheck.c"

void print_expression(expression_node e);
void print_leaves(infix_node* node);

int main(int argc, char **argv) {
    if (argc < 2) {
        compiler_fprintf(stderr, "Usage: '%s' <file>", argv[0]);
        exit(GENERIC_COMPILER_ERROR);
    }

    // handle compiler testing arguments (intended for internal use only)
    bool pretty_print_and_exit = false;
    bool typecheck_and_exit = false;
    int arg_idx = 1;
    while (arg_idx < argc - 1) {
        char *arg = argv[arg_idx];
        // Not all valid ASTs pass type checking. Allow parser test cases to
        // continue working even as later compiler phases are implemented
        if (cstring_equals_cstring(arg, "--pretty-print-and-exit")) {
            pretty_print_and_exit = true;
        } else if (cstring_equals_cstring(arg, "--typecheck-and-exit")) {
            typecheck_and_exit = true;
        }
        arg_idx += 1;
    }
    char *main_file_name = argv[arg_idx];
    string contents = read_entire_file(main_file_name);

    token_array tokens = lex_entire_file(contents);

	statement_node *first_statement = parse_entire_file(&tokens, main_file_name, contents);

    if (pretty_print_and_exit) {
        pretty_print_entire_file(first_statement);
        return 0;
    }

    statement_node *current_statement = first_statement;
    while (current_statement != NULL) {
        typecheck_top_level_statement(current_statement, typecheck_and_exit);
        current_statement = current_statement->next;
    }
    if (typecheck_and_exit) {
        return 0;
    }


	return 0;
}
