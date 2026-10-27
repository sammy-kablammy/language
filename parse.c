typedef struct parser_state {
	token_array tokens;
    uint64_t pos;
    token current_token;
} parser_state;

// To keep current_token in sync, prefer this function instead of accessing
// parser state directly.
void parser_advance(parser_state *state) {
    state->pos += 1;
    if (state->pos > state->tokens.length) {
        panic("Advancing parser to token pos %lu would go out of bounds", state->pos);
    }
    state->current_token = state->tokens.buf[state->pos];
}

void parser_eat_newlines(parser_state *state) {
    while (state->pos < state->tokens.length && state->current_token.kind == TOKEN_END_OF_LINE) {
        parser_advance(state);
    }
}

typedef struct assignment_node assignment_node;
typedef struct statement_node statement_node;
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

struct statement_node {
    enum {
        ASSIGNMENT,
        RETURN,
    } kind;

    union {
        assignment_node assignment;
        // This one's a pointer because a return statement may or may not have
        // an expression with it. NULL pointer represents plain return.
        expression_node *maybe_return_expression;
    } value;

    statement_node *next;
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

assignment_node parse_assignment(parser_state* state);
statement_node *parse_statement(parser_state* state);
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

statement_node* parse_entire_file(token_array tokens) {
	parser_state state = {
        .tokens = tokens,
        .pos = 0,
        .current_token = tokens.buf[0],
    };

    parser_eat_newlines(&state);

    statement_node *first_statement = NULL;
	if (state.pos < state.tokens.length) {
		switch (state.current_token.kind) {
			case TOKEN_RETURN: // fall through
			case TOKEN_IDENTIFIER:
				first_statement = parse_statement(&state);
				break;
			default:
				panic("uncaught syntax at token %ld\n", state.pos);
		}
        parser_eat_newlines(&state);
    }

    statement_node *current_statement = first_statement;
	while (state.pos < state.tokens.length) {
        switch (state.current_token.kind) {
			case TOKEN_RETURN: // fall through
			case TOKEN_IDENTIFIER:
				current_statement->next = parse_statement(&state);
				current_statement = current_statement->next;
				break;
			case TOKEN_END_OF_LINE:
				parser_eat_newlines(&state);
				break;
			default:
				panic("uncaught syntax at token %ld\n", state.pos);
		}
        parser_eat_newlines(&state);
	}
	return first_statement;
}

assignment_node parse_assignment(parser_state* state) {
    if (state->current_token.kind != TOKEN_IDENTIFIER) {
        panic("parse_assignment currently expects left side of '=' to be an identifier");
    }
    assignment_node assignment = { 0 };
	assignment.identifier = state->current_token.value.identifier;
    parser_advance(state);
	if (state->current_token.kind != TOKEN_ASSIGN) {
		print_token(state->current_token);
		panic(" receieved, expected operator instead");
	}
    parser_advance(state);
	assignment.expression = parse_expression(state, MIN_PREC);
	return assignment;
}

statement_node *parse_statement(parser_state *state) {
    statement_node *stmt = xalloc(sizeof(statement_node));
    switch (state->current_token.kind) {
        case TOKEN_RETURN:
            stmt->kind = RETURN;
            parser_advance(state);
            if (state->current_token.kind == TOKEN_END_OF_LINE) {
                stmt->value.maybe_return_expression = NULL;
            } else {
                stmt->value.maybe_return_expression = parse_expression(state, MIN_PREC);
            }
            break;
        case TOKEN_IDENTIFIER:
            stmt->kind = ASSIGNMENT;
            stmt->value.assignment = parse_assignment(state);
            break;
        default:
            panic("Unrecognized token kind (%d) in parse_statement", state->current_token.kind);
    }
    return stmt;
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

    while (is_operator(state->current_token) && prev_prec < operator_prec(state->current_token)) {
        token op = state->current_token;
        parser_advance(state);
        expression_node *right = parse_expression(state, operator_prec(op));
        left = create_infix_expression(left, op, right);
    }

    return left;
}

expression_node *parse_leaf(parser_state* state) {
	expression_node *leaf = xalloc(sizeof(expression_node));
	if (state->current_token.kind == TOKEN_INTEGER) {
		leaf->kind = INT;
		leaf->value.integer = state->current_token.value.integer;
	}
	else if (state->current_token.kind == TOKEN_IDENTIFIER) {
		leaf->kind = IDENTIFIER;
		leaf->value.identifier = state->current_token.value.identifier;
	}
	else {
		panic("unrecognized token kind in parse_leaf, got=%d\n", state->current_token.kind);
	}
    parser_advance(state);
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
