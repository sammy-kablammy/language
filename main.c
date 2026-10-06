// This project is a unity build / single compilation unit build.
#include "platform.c"
#include "lex.c"
#include "parse.c"

int simulate_arithmetic(expression_node e);
bool are_both_leaves(infix_node* node);
int evaluate_leaves(int left, int right, token operator);
void print_expression(expression_node e);
void print_leaves(infix_node* node);

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
	printf("%d\n", simulate_arithmetic(list[0]->expression));
	/*
	for each top level statement {
        codegen(statement);
    }
	*/
	
	free(list);
	return 0;
}
int simulate_arithmetic(expression_node e) {
	int left = 0;
	int right = 0;
	if (e.value.infix->left.kind == INFIX) {
		left = simulate_arithmetic(e.value.infix->left);
	}
	else if (e.value.infix->left.kind == INT) {
		left = e.value.infix->left.value.integer;
	}
	if (e.value.infix->right.kind == INFIX) {
		right = simulate_arithmetic(e.value.infix->right);
	}
	else if (e.value.infix->right.kind == INT) {
		right = e.value.infix->right.value.integer;
	}
	return evaluate_leaves(left, right, e.value.infix->operator);
}

bool are_both_leaves(infix_node* node) {
	if ((node->left.kind == INT || node->left.kind == IDENTIFIER)
		&& (node->right.kind == INT || node->right.kind == IDENTIFIER)) return true;
	return false;
}

int evaluate_leaves(int left, int right, token operator) {
	switch(operator.kind) {
		case TOKEN_PLUS:
			printf("%d + %d\n", left, right);
			return left + right;
		case TOKEN_MINUS:
			printf("%d - %d\n", left, right);
			return left - right;
		case TOKEN_STAR:
			printf("%d * %d\n", left, right);
			return left * right;
		case TOKEN_SLASH:
			printf("%d / %d\n", left, right);
			return left / right;
		default:
			panic("what happened\n");
			exit(1);
	}
}
