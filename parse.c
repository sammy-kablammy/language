typedef struct parser_state {
    uint64_t position_in_token_stream;
} parser_state;

typedef struct expression_node expression_node;
typedef struct infix_node infix_node;

typedef struct syntax_tree_node {
	int dummy;
} syntax_tree_node;

struct expression_node {
    enum {
        INFIX,
        INT,
        IDENTIFIER,
    } kind;

    union {
        int integer;
        string identifier;
        infix_node *infix;
    } value;
};

struct infix_node {
    expression_node *left;
    expression_node *right;
    token operator;
};

expression_node* parse_expression(token_array tokens, parser_state* state);
expression_node* parse_leaf(token t);
int is_operator(token t);
// A program consists of statements
// a statement can be either a declaration, an assignment or a procedure call
//       x: int = 10
//       x = 100
//       print(x)
syntax_tree_node* parse_entire_file(token_array tokens) {
	syntax_tree_node* node = xalloc(sizeof(syntax_tree_node));
	parser_state* state = xalloc(sizeof(parser_state));
	parse_expression(tokens, state);
	return node;
}
// expression examples:
//       10 + 20			infix expression
//       -5					number
//       y					identifier
expression_node* parse_expression(token_array tokens, parser_state* state) {
	expression_node* leaf = parse_leaf(tokens.buf[state->position_in_token_stream]);
	state->position_in_token_stream++;
	// We have an operator to capture and a right side
	if (is_operator(tokens.buf[state->position_in_token_stream])) {
		// An operator means this is an in-fix expression
		infix_node* infix = xalloc(sizeof(infix_node));
		infix->operator = tokens.buf[state->position_in_token_stream];
		state->position_in_token_stream++;
		infix->left = leaf;
		infix->right = parse_expression(tokens, state);
		expression_node* expression = xalloc(sizeof(expression_node));
		expression->value.infix = infix;
		expression->kind = INFIX;
		return expression;
	}
	// There is not an operator, meaning this is a leaf node. Return the leaf as an expression
	else {
		return leaf;
	}
}

expression_node* parse_leaf(token t) {
	expression_node* leaf = xalloc(sizeof(expression_node));
	if (t.kind == TOKEN_INTEGER) {
		leaf->kind = INT;
		leaf->value.integer = t.value.integer;
	}
	else if (t.kind == TOKEN_IDENTIFIER) {
		leaf->kind = IDENTIFIER;
		leaf->value.identifier = t.value.identifier;
	}
	else {
		printf("unrecognized syntax\n");
	}
	return leaf;
}

int is_operator(token t) {
	switch(t.kind) {
		case TOKEN_PLUS:
			return 1;
		case TOKEN_MINUS:
			return 1;
		case TOKEN_STAR:
			return 1;
		case TOKEN_SLASH:
			return 1;
		default:
			return 0;
	}
}