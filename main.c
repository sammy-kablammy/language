// This project is a unity build / single compilation unit build.
#include "platform.c"
#include "lex.c"

int main(int argc, char **argv) {
    if (argc != 2) {
        panic("Usage: '%s <file>'", argv[0]);
    }
    string contents = read_entire_file(argv[1]);
    (void)contents;

    token_array tokens = lex_entire_file(contents);

    debug_print_tokens(tokens);

    // Parse one entire file into multiple syntax trees, one per top-level declaration
    // syntax_tree_node *node = parse_entire_file(tokens);

/*
    read input file

    parse each top level statement into its own thing

    syntax_tree top_level_statements[];

    for each top level statement {
        codegen(statement);
    }
*/

}
