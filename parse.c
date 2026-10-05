typedef struct parser_state {
	token_array tokens;
    uint64_t pos;
} parser_state;

typedef struct assignment_node assignment_node;
typedef struct expression_node expression_node;
typedef struct infix_node infix_node;

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

struct assignment_node {
	string identifier;
	expression_node expression;
};

struct infix_node {
    expression_node left;
    expression_node right;
    token operator;
};

assignment_node* parse_assignment(parser_state* state);
expression_node parse_expression(parser_state* state, int prev_precedence);
expression_node parse_leaf(parser_state* state);
int operator_prec(token t);
void print_token(token t);
bool is_operator(token t);

enum {
	MIN_PREC,
	ADD_PREC,
	MULT_PREC
};

int operator_prec(token t) {
	switch (t.kind) {
		case TOKEN_PLUS:
		case TOKEN_MINUS:
			return ADD_PREC;
		case TOKEN_STAR:
		case TOKEN_SLASH:
			return MULT_PREC;
		default:
			print_token(t);
			panic(" is not a recognizable operator");
			exit(1);
	}
}

assignment_node** parse_entire_file(token_array tokens) {
	parser_state* state = xalloc(sizeof(parser_state));
	state->tokens = tokens;
	assignment_node** assignments = xalloc(sizeof(assignment_node*) * 1000);
	int assignment_index = 0;
	while (state->pos+1 < state->tokens.length) {
		// store statement inside larger structure
		assignments[assignment_index] = parse_assignment(state);
		expression_node e = assignments[assignment_index]->expression;
		while (true) {
			if (e.kind == INT) {
				printf("%d ", e.value.integer);
				break;
			}
			else if (e.kind == IDENTIFIER) {
				print_string(e.value.identifier);
				printf(" ");
				break;
			}
			else if (e.kind == INFIX) {
				if (e.value.infix->left.kind == IDENTIFIER) {
				print_string(e.value.infix->left.value.identifier);
				printf(" ");
				}
				else if (e.value.infix->left.kind == INT) {
					printf("%d ", e.value.infix->left.value.integer);
				}
				printf("%c ", e.value.infix->operator.kind);
				e = e.value.infix->right;
			}
		}
		printf("\n");
		// read current token and expect it to be a newline
		if (state->tokens.buf[state->pos].kind != TOKEN_END_OF_LINE) {
			// send error
			if (is_operator(state->tokens.buf[state->pos])) {
				printf("newline expected, %c found instead\n", state->tokens.buf[state->pos].kind);
			}
			else if (state->tokens.buf[state->pos].kind == TOKEN_ASSIGN) {
				printf("newline expected, %c found instead\n", state->tokens.buf[state->pos].kind);
			}
			else if (state->tokens.buf[state->pos].kind == TOKEN_IDENTIFIER) {
				printf("newline expected, ");
				print_string(state->tokens.buf[state->pos].value.identifier);
				printf(" found instead\n");
			}
			else if (state->tokens.buf[state->pos].kind == TOKEN_INTEGER) {
				printf("newline expected, %ld found instead\n", state->tokens.buf[state->pos].value.integer);
			}
			else {
				printf("unrecognized or no token found instead\n");
			}
			break;
		}
		else {
			state->pos++;
		}
		// increment our position in larger structure of statement storage
		assignment_index++;
		if (assignment_index > 1000) {
			printf("something bad happened\n");
			break;
		}
	}
	free(state);
	return assignments;
}

assignment_node* parse_assignment(parser_state* state) {
	assignment_node* assignment = xalloc(sizeof(assignment_node));
	// we are expecting an assignment, so there should be at least 2 more tokens,
	// and the second token should be an assignment token
	
	
	assignment->identifier = state->tokens.buf[state->pos].value.identifier;
	print_token(state->tokens.buf[state->pos]);
	state->pos += 1;
	if (state->tokens.buf[state->pos].kind != TOKEN_ASSIGN) {
		print_token(state->tokens.buf[state->pos]);
		panic(" receieved, expected operator instead");
	}
	print_token(state->tokens.buf[state->pos]);
	state->pos += 1;
	assignment->expression = parse_expression(state, MIN_PREC);
	return assignment;
}

expression_node parse_expression(parser_state* state, int prev_precedence) {
	expression_node left = parse_leaf(state);
	token potential_op = state->tokens.buf[state->pos];
	while (is_operator(potential_op) && prev_precedence < operator_prec(potential_op)) {
		int next_precedence = operator_prec(potential_op);
		expression_node parent = {.kind=INFIX};
		parent.value.infix = xalloc(sizeof(infix_node));
		parent.value.infix->operator = potential_op;
		print_token(state->tokens.buf[state->pos]);
		state->pos++;
		potential_op = state->tokens.buf[state->pos];
		parent.value.infix->left = left;
		parent.value.infix->right = parse_expression(state, next_precedence);
		left = parent;
	}
    return left;
}

expression_node parse_leaf(parser_state* state) {
	expression_node leaf;
	token t = state->tokens.buf[state->pos];
	if (t.kind == TOKEN_INTEGER) {
		leaf.kind = INT;
		leaf.value.integer = t.value.integer;
		print_token(state->tokens.buf[state->pos]);
		state->pos++;
	}
	else if (t.kind == TOKEN_IDENTIFIER) {
		leaf.kind = IDENTIFIER;
		leaf.value.identifier = t.value.identifier;
		print_token(state->tokens.buf[state->pos]);
		state->pos++;
	}
	else {
		panic("unrecognized syntax\n");
	}
	return leaf;
}

void print_token(token t) {
	switch (t.kind) {
		case TOKEN_ASSIGN: // fall through
		case TOKEN_PLUS: // fall through
        case TOKEN_MINUS: // fall through
        case TOKEN_STAR: // fall through 
        case TOKEN_SLASH: // fall through
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

bool is_operator(token t) {
	switch(t.kind) {
		case TOKEN_PLUS: // fall through
		case TOKEN_MINUS: // fall through
		case TOKEN_STAR: // fall through
		case TOKEN_SLASH:
			return true;
		default:
			return false;
	}
}