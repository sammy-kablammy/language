// This project is a unity build / single compilation unit build.
#include "platform.c"
#include "lex.c"
#include "parse.c"
#include "pretty_print.c"

void print_expression(expression_node e);
void print_leaves(infix_node* node);

int main(int argc, char **argv) {
    if (argc < 2) {
        panic("Usage: '%s <file>'", argv[0]);
    }

    // handle compiler testing arguments (intended for internal use only)
    bool exit_after_pretty_print = false;
    int arg_idx = 1;
    while (arg_idx < argc - 1) {
        char *arg = argv[arg_idx];
        // Not all valid ASTs pass type checking. Allow parser test cases to
        // continue working even as later compiler phases are implemented
        if (cstring_equals_cstring(arg, "--pretty-print-and-exit")) {
            exit_after_pretty_print = true;
        }
        arg_idx += 1;
    }
    string contents = read_entire_file(argv[arg_idx]);

    token_array tokens = lex_entire_file(contents);

	statement_node *first_statement = parse_entire_file(tokens);

    statement_node *current_statement = first_statement;
    while (current_statement != NULL) {
        pretty_print_statement(current_statement);
        current_statement = current_statement->next;
    }
    if (exit_after_pretty_print) {
        return 0;
    }

	return 0;
}
