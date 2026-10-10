// This project is a unity build / single compilation unit build.
#include "platform.c"
#include "lex.c"
#include "parse.c"
// #include "simulate.c"
#include "pretty_print.c"

void print_expression(expression_node e);
void print_leaves(infix_node* node);

int main(int argc, char **argv) {
    if (argc != 2) {
        panic("Usage: '%s <file>'", argv[0]);
    }
    string contents = read_entire_file(argv[1]);
    token_array tokens = lex_entire_file(contents);

	statement_node *first_statement = parse_entire_file(tokens);

    // statement_node *current_statement = first_statement;
    // while (current_statement != NULL) {
    //     if (current_statement->kind == ASSIGNMENT) {
    //         int result = simulate_arithmetic(current_statement->value.assignment.expression);
    //         printf("%d\n", result);
    //         printf("\n");
    //     }
    //     current_statement = current_statement->next;
    // }

    statement_node *current_statement = first_statement;
    while (current_statement != NULL) {
        pretty_print_statement(current_statement);
        current_statement = current_statement->next;
    }

	return 0;
}
