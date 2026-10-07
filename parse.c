typedef struct parser_state {
	token_array tokens;
    uint64_t pos;
} parser_state;

typedef struct assignment_node assignment_node;
typedef struct expression_node expression_node;
typedef struct infix_node infix_node;

struct infix_node {
    expression_node *left;
    expression_node *right;
    token operator;
};

struct assignment_node {
	string identifier;
	expression_node *expression;
};

struct expression_node {
    enum {
        INFIX,
        INT,
        IDENTIFIER,
    } kind;

    union {
        int integer;
        string identifier;
        infix_node infix;
    } value;
};


// // TODO make syntax_tree_node with kind == EXPRESSION_INFIX
// struct syntax_tree_node {
// };

assignment_node *parse_assignment(parser_state* state);
expression_node *parse_expression(parser_state* state, int prev_prec);
expression_node *parse_leaf(parser_state* state);
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
		// read current token and expect it to be a newline
		switch (state->tokens.buf[state->pos].kind) {
			case TOKEN_ASSIGN: // fall through
			case TOKEN_PLUS: // fall through
			case TOKEN_MINUS: // fall through
			case TOKEN_STAR: // fall through
			case TOKEN_SLASH: // fall through
			case TOKEN_INTEGER: // fall through
			case TOKEN_IDENTIFIER:
				printf("newline expected, ");
				print_token(state->tokens.buf[state->pos]);
				printf(" found instead\n");
				panic("\n");
			case TOKEN_END_OF_LINE:
				state->pos += 1;
				break;
			default:
				printf("potentially reached end of file, or unrecognized token\n");
				break;
		}
		// increment our position in larger structure of statement storage
		assignment_index += 1;
		if (assignment_index > 20) {
			printf("something bad happened, make sure less than 20 assignments have been parsed\n");
			break;
		}
	}
	free(state);
	return assignments;
}

assignment_node *parse_assignment(parser_state* state) {
	assignment_node* assignment = xalloc(sizeof(assignment_node));
	assignment->identifier = state->tokens.buf[state->pos].value.identifier;
	state->pos += 1;
	if (state->tokens.buf[state->pos].kind != TOKEN_ASSIGN) {
		print_token(state->tokens.buf[state->pos]);
		panic(" receieved, expected operator instead");
	}
	state->pos += 1;
	assignment->expression = parse_expression(state, MIN_PREC);
	return assignment;
}

expression_node *create_infix_expression(expression_node *left, token op, expression_node *right) {
	expression_node* expression = xalloc(sizeof(expression_node));
    expression->kind = INFIX;
    expression->value.infix.left = left;
    expression->value.infix.operator = op;
    expression->value.infix.right = right;
    return expression;
}

expression_node *parse_expression(parser_state* state, int prev_prec) {
	expression_node *left = parse_leaf(state);
	token potential_op = state->tokens.buf[state->pos];

    while (is_operator(potential_op) && prev_prec < operator_prec(potential_op)) {
        state->pos += 1;
        expression_node *right = parse_expression(state, operator_prec(potential_op));
        left = create_infix_expression(left, potential_op, right);
        potential_op = state->tokens.buf[state->pos];
    }

    return left;
}

expression_node *parse_leaf(parser_state* state) {
	expression_node *leaf = xalloc(sizeof(expression_node));
	token t = state->tokens.buf[state->pos];
	if (t.kind == TOKEN_INTEGER) {
		leaf->kind = INT;
		leaf->value.integer = t.value.integer;
		state->pos++;
	}
	else if (t.kind == TOKEN_IDENTIFIER) {
		leaf->kind = IDENTIFIER;
		leaf->value.identifier = t.value.identifier;
		state->pos++;
	}
	else {
		panic("unrecognized token kind in parse_leaf, got=%d\n", t.kind);
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
			printf("%c ", t.kind);
			break;
        case TOKEN_END_OF_LINE:
			printf("newline ");
			break;
		case TOKEN_IDENTIFIER:
			printf("'");
			print_string(t.value.identifier);
			printf("' ");
			break;
		case TOKEN_INTEGER:
			printf("'%ld' ", t.value.integer);
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
