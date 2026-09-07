// This project is a unity build / single compilation unit build.
#include "platform.c"

int main(int argc, char **argv) {
    if (argc != 2) {
        panic("Usage: '%s <file>'", argv[0]);
    }
    string contents = read_entire_file(argv[1]);
    (void)contents;

/*
    read input file

    parse each top level statement into its own thing

    syntax_tree top_level_statements[];

    for each top level statement {
        codegen(statement);
    }
*/

}
