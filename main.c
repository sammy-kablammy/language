// This project is a unity build / single compilation unit build.
#include "platform.c"
#include "lex.c"
#include "parse.c"

int simulate_arithmetic(expression_node e);
bool are_both_leaves(infix_node* node);
int evaluate_leaves(infix_node* node);
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
	print_expression(list[0]->expression);
	//printf("%d\n", simulate_arithmetic(list[0]->expression));
	
	/*
	for each top level statement {
        codegen(statement);
    }
	*/
	
	free(list);
	return 0;
}

int simulate_arithmetic(expression_node e) {
	// look at the left side of expression, if it is a nested expression, set that left side = to a 
	// recursed call
	if (e.value.infix->left.kind != INT && e.value.infix->left.kind != IDENTIFIER) {
		// technically change the kind to the (eventually) evaluated result
		e.value.infix->left.value.integer = simulate_arithmetic(e.value.infix->left);
		e.value.infix->left.kind = INT;
	}
	// look at the right side of expression, if it is a nested expression, set that right side = to a
	// recursed call
	if (e.value.infix->right.kind != INT && e.value.infix->right.kind != IDENTIFIER) {
		// technically change the kind to the (eventually) evaluated result
		e.value.infix->right.value.integer = simulate_arithmetic(e.value.infix->right);
		e.value.infix->right.kind = INT;
	}
	// once both have been evaluated, it should be able to be evaluated as leaves
	return evaluate_leaves(e.value.infix);
}

bool are_both_leaves(infix_node* node) {
	if ((node->left.kind == INT || node->left.kind == IDENTIFIER)
		&& (node->right.kind == INT || node->right.kind == IDENTIFIER)) return true;
	return false;
}

int evaluate_leaves(infix_node* node) {
	switch(node->operator.kind) {
		case TOKEN_PLUS:
			printf("%d + %d\n", node->left.value.integer, node->right.value.integer);
			return node->left.value.integer + node->right.value.integer;
		case TOKEN_MINUS:
			printf("%d + %d\n", node->left.value.integer, node->right.value.integer);
			return node->left.value.integer - node->right.value.integer;
		case TOKEN_STAR:
			printf("%d + %d\n", node->left.value.integer, node->right.value.integer);
			return node->left.value.integer * node->right.value.integer;
		case TOKEN_SLASH:
			printf("%d + %d\n", node->left.value.integer, node->right.value.integer);
			return node->left.value.integer / node->right.value.integer;
		default:
			panic("wtf happened\n");
			exit(1);
	}
}

void print_expression(expression_node e) {
	// look at the left side of expression, if it is a nested expression, print a bracket,
	// and then recursively call to print the inside of the expression
	if (e.value.infix->left.kind == INFIX) {
		printf("{ ");
		print_expression(e.value.infix->left);
		printf(" } ");
	}
	// look at the right side of expression, if it is a nested expression, print a bracket,
	// and then recursively call to print the inside of the expression
	if (e.value.infix->right.kind != INFIX) {
		printf("{ ");
		print_expression(e.value.infix->right);
		printf(" } ");
	}
	// once both have been evaluated, it should be able to be evaluated as leaves
	print_leaves(e.value.infix);
}

void print_leaves(infix_node* node) {
	switch(node->operator.kind) {
		case TOKEN_PLUS:
			printf("%d + %d }\n", node->left.value.integer, node->right.value.integer);
			return;
		case TOKEN_MINUS:
			printf("%d + %d }\n", node->left.value.integer, node->right.value.integer);
			return;
		case TOKEN_STAR:
			printf("%d + %d }\n", node->left.value.integer, node->right.value.integer);
			return;
		case TOKEN_SLASH:
			printf("%d + %d }\n", node->left.value.integer, node->right.value.integer);
			return;
		default:
			panic("wtf happened\n");
			exit(1);
	}
}