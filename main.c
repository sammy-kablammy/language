// This project is a unity build / single compilation unit build.
#include "platform.c"
#include "arena.c"
#include "lex.c"
#include "parse.c"

void free_assignments(assignment_node** list);

int main(int argc, char **argv) {
    if (argc != 2) {
        panic("Usage: '%s <file>'", argv[0]);
    }
	//  read input file
    string contents = read_entire_file(argv[1]);
    (void)contents;
    token_array tokens = lex_entire_file(contents);

    // Parse one entire file into multiple syntax trees, one per top-level declaration
    // syntax_tree top_level_statements[];
	assignment_node** list = parse_entire_file(tokens);
	
	/*
	for each top level statement {
        codegen(statement);
    }
	*/
	
	free(list);
	return 0;
}