// This project is a unity build / single compilation unit build.
#include "platform.c"
#include "lex.c"
#include "parse.c"

void free_assignments(assignment_node** list);

int main(int argc, char **argv) {
    if (argc != 2) {
        panic("Usage: '%s <file>'", argv[0]);
    }
    string contents = read_entire_file(argv[1]);
    (void)contents;
    token_array tokens = lex_entire_file(contents);
    debug_print_tokens(tokens);

    // Parse one entire file into multiple syntax trees, one per top-level declaration
    //syntax_tree_node *node = parse_entire_file(tokens);
	assignment_node** list = parse_file_debugger(tokens);
	free_assignments(list);
	return 0;
/*
    read input file

    parse each top level statement into its own thing

    syntax_tree top_level_statements[];

    for each top level statement {
        codegen(statement);
    }
*/

}

void free_assignments(assignment_node** list) {
	int assignment_index = 0;
	while (list[assignment_index] != NULL) {
		expression_node* e = list[assignment_index]->expression;
		printf("assignment #%d\n", assignment_index+1);
		while (e != NULL && e->kind == INFIX) {
			printf("%p\n", (void *)e);
			printf("%p\n",(void *)e->value.infix);
			printf("%p\n\n\n", (void *)e->value.infix->left);
			if (e->value.infix->left->kind == IDENTIFIER) {
				print_string(e->value.infix->left->value.identifier);
				printf(" ");
			}
			else if (e->value.infix->left->kind == INT) {
				/**
				QUARANTINE ZONE
				this causes a segfault and i genuinely cant figure out why
				the print statements above showing the pointers are not null has caused me
				much much confusion.
				im sure i just need to be more strict about ensuring non null values but i literally
				dont know how to so im giving up. please god help me.
				
				int temp = e->value.infix->left->value.integer;
				printf("%p is the address of the int\n", (void *)&temp);
				printf("%d ", temp);
				**/
			}
			printf("%c ", e->value.infix->operator.kind);
			expression_node* temp = e;
			free(e->value.infix->left);
			e = e->value.infix->right;
			free(temp);
		}
		if (e->value.infix->left->kind == IDENTIFIER) {
			print_string(e->value.infix->left->value.identifier);
			printf(" ");
		}
		else if (e->value.infix->left->kind == INT) {
			printf("%d ", e->value.infix->left->value.integer);
		}
		printf("%c ", e->value.infix->operator.kind);
		free(e);
		printf("\n");
		assignment_index++;
	}
	free(list);
}