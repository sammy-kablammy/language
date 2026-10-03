typedef struct parser_state {
    uint64_t position_in_token_stream;
} parser_state;

typedef struct statement_node statement_node;
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
expression_node* parse_leaf(token t, parser_state* state);
void print_token(token t);
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

expression_node** parse_file_debugger(token_array tokens) {
	parser_state* state = xalloc(sizeof(parser_state));
	expression_node** expressions = xalloc(sizeof(expression_node*) * 1000);
	int expression_index = 0;
	while (state->position_in_token_stream+1 < tokens.length) {
		// store statement inside larger structure
		expressions[expression_index] = parse_expression(tokens, state);
		expression_node* e = expressions[expression_index];
		while (e != NULL) {
			if (e->kind == INT) {
				printf("%d ", e->value.integer);
				break;
			}
			else if (e->kind == IDENTIFIER) {
				printf("%s ", e->value.identifier.buf);
				break;
			}
			else if (e->kind == INFIX) {
				printf("%d %c ", e->value.infix->left->value.integer, e->value.infix->operator.kind);
				e = e->value.infix->right;
			}
		}
		printf("\n");
		// read current token and expect it to be a newline
		if (tokens.buf[state->position_in_token_stream].kind != TOKEN_END_OF_LINE) {
			// send error
			if (is_operator(tokens.buf[state->position_in_token_stream])) {
				printf("newline expected, %c found instead\n", tokens.buf[state->position_in_token_stream].kind);
			}
			else if (tokens.buf[state->position_in_token_stream].kind == TOKEN_ASSIGN) {
				printf("newline expected, %c found instead\n", tokens.buf[state->position_in_token_stream].kind);
			}
			else if (tokens.buf[state->position_in_token_stream].kind == TOKEN_IDENTIFIER) {
				printf("newline expected, ");
				print_string(tokens.buf[state->position_in_token_stream].value.identifier);
				printf(" found instead\n");
			}
			else if (tokens.buf[state->position_in_token_stream].kind == TOKEN_INTEGER) {
				printf("newline expected, %ld found instead\n", tokens.buf[state->position_in_token_stream].value.integer);
			}
			else {
				printf("unrecognized or no token found instead\n");
				printf("pos in token stream: %lu\n", state->position_in_token_stream);
				printf("token ascii value: %d\n", tokens.buf[state->position_in_token_stream].kind);
			}
			break;
		}
		else {
			state->position_in_token_stream++;
		}
		// increment our position in larger structure of statement storage
		expression_index++;
		if (expression_index > 1000) {
			printf("something bad happened\n");
			break;
		}
	}
	//expression_node* expression = parse_expression(tokens, state);
	free(state);
	return expressions;
}
// expression examples:
//       10 + 20			infix expression
//       -5					number
//       y					identifier
expression_node* parse_expression(token_array tokens, parser_state* state) {
	expression_node* leaf = parse_leaf(tokens.buf[state->position_in_token_stream], state);
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

expression_node* parse_leaf(token t, parser_state* state) {
	print_token(t);
	expression_node* leaf = xalloc(sizeof(expression_node));
	if (t.kind == TOKEN_INTEGER) {
		leaf->kind = INT;
		leaf->value.integer = t.value.integer;
		state->position_in_token_stream++;
	}
	else if (t.kind == TOKEN_IDENTIFIER) {
		leaf->kind = IDENTIFIER;
		leaf->value.identifier = t.value.identifier;
		state->position_in_token_stream++;
	}
	else {
		
		printf("unrecognized syntax\n");
	}
	return leaf;
}

void print_token(token t) {
	switch (t.kind) {
		case TOKEN_ASSIGN: // fall through
		case TOKEN_COLON: // fall through
		case TOKEN_PLUS: // fall through
        case TOKEN_MINUS: // fall through
        case TOKEN_STAR: // fall through 
        case TOKEN_SLASH: // fall through
		case TOKEN_L_PARENTHESIS: // fall through
		case TOKEN_R_PARENTHESIS: // fall through
			printf("token read: %c ", t.kind);
			break;
        case TOKEN_END_OF_LINE:
			printf("token read: newline ");
			break;
		case TOKEN_IDENTIFIER:
			printf("token read: '");
			print_string(t.value.identifier);
			printf("' ");
			break;
		case TOKEN_INTEGER:
			printf("token read: '%ld' ", t.value.integer);
			break;
		default:
			printf("token read: i have no clue ");
			break;
	}
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