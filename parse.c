typedef struct parser_state {
	token_array *tokens;
    uint64_t pos;
    token current_token;
    char *file_name; // (used for error handling)
    string file_contents; // (used for error handling)
} parser_state;

__attribute__((noreturn)) void parser_error(parser_state *state, const char *error_message, ...) {
    va_list args;
    va_start(args, error_message);
    char error_buffer[10000] = { 0 };
    char *buf = error_buffer;

    string source_code = token_to_source_code_line(state->file_contents, state->current_token);
    uint64_t line_number = token_to_line_number(state->file_contents, state->current_token);
    uint64_t column = state->current_token.file_position - (uintptr_t)source_code.buf + (uintptr_t)state->file_contents.buf;

    buf += compiler_sprintf(buf, "%s%s%s ", ANSI_BOLD, state->file_name, ANSI_RESET);
    buf += compiler_sprintf(buf, "%s(%d:%d)%s", ANSI_FAINT, line_number, column, ANSI_RESET);
    buf += compiler_sprintf(buf, "%s%s error:%s ", ANSI_RED, ANSI_BOLD, ANSI_RESET);
    buf += compiler_vsprintf(buf, error_message, args);
    buf += compiler_sprintf(buf, "\n");

    buf += compiler_sprintf(buf, "  %s%s%d |%s %S", ANSI_FAINT, ANSI_BOLD, line_number, ANSI_RESET, source_code);
    buf += compiler_sprintf(buf, "\n");

    buf += compiler_sprintf(buf, "  ");
    for (int digit = line_number; digit > 0; digit /= 10) {
        buf += compiler_sprintf(buf, " ");
    }
    buf += compiler_sprintf(buf, " %s%s| ", ANSI_FAINT, ANSI_BOLD);
    for (uint64_t i = 0; i < column; i++) {
        buf += compiler_sprintf(buf, " ");
    }
    buf += compiler_sprintf(buf, "^%s", ANSI_RESET);
    buf += compiler_sprintf(buf, "\n");

    buf += compiler_sprintf(buf, "\0");
    compiler_fprintf(stderr, "%s", error_buffer);
    va_end(args);
    exit(PARSE_ERROR);
}

// To keep current_token in sync, prefer this function instead of accessing
// parser state directly.
void parser_advance(parser_state *state) {
    state->pos += 1;
    if (state->pos > state->tokens->length) {
        panic("Advancing parser to token pos %lu would go out of bounds", state->pos);
    }
    state->current_token = token_array_get(state->tokens, state->pos);
}

void parser_eat_newlines(parser_state *state) {
    while (state->pos < state->tokens->length && state->current_token.kind == TOKEN_END_OF_LINE) {
        parser_advance(state);
    }
}

typedef struct statement_node statement_node;
typedef struct expression_node expression_node;
typedef struct infix_node infix_node;

struct infix_node {
    expression_node *left;
    expression_node *right;
    token operator;
};

typedef struct declaration_node {
    string identifier;
    string data_type;
    expression_node *expression;
} declaration_node;

typedef struct assignment_node {
    string lhs;
    expression_node *expression;
} assignment_node;

typedef struct while_loop_node {
    expression_node *condition;
    statement_node *body;
} while_loop_node;

typedef struct conditional_node {
    expression_node *condition;
    statement_node *body;
    statement_node *alt_body; // not guaranteed to be non-null
} conditional_node;
typedef struct function_node {
    statement_node *body;
} function_node;

struct statement_node {
    enum {
        DECLARATION,
        ASSIGNMENT,
        WHILE_LOOP,
		CONDITIONAL,
        RETURN,
        BREAK,
        CONTINUE,
    } kind;

    union {
        declaration_node declaration;
        assignment_node assignment;
        while_loop_node while_loop;
        conditional_node conditional;
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
        BOOL,
        INLINE_FUNCTION,
    } kind;

    union {
        int integer;
        string identifier;
        infix_node infix;
        bool bool;
        function_node function_declaration;
    } value;
};

assignment_node parse_assignment(parser_state* state);
statement_node *parse_statement(parser_state* state);
expression_node *parse_expression(parser_state* state, int prev_prec);
expression_node *parse_leaf(parser_state* state);
int operator_prec(token t);
bool is_operator(token t);

enum {
	MIN_PREC,
	RELATIONAL_PREC,
	ADD_PREC,
	MULT_PREC,
	DOT_PREC
};

int operator_prec(token t) {
	switch (t.kind) {
		case TOKEN_PLUS:
		case TOKEN_MINUS:
			return ADD_PREC;
		case TOKEN_STAR:
		case TOKEN_SLASH:
			return MULT_PREC;
		case TOKEN_DOT:
			return DOT_PREC;
		case TOKEN_LESS_THAN:
		case TOKEN_LESS_THAN_OR_EQUAL_TO:
		case TOKEN_GREATER_THAN:
		case TOKEN_GREATER_THAN_OR_EQUAL_TO:
		case TOKEN_EQUAL_TO:
		case TOKEN_NOT_EQUAL_TO:
			return RELATIONAL_PREC;
		default:
            compiler_fprintf(stderr, "%T", t);
			panic(" is not a recognizable operator");
	}
}

// (This looks kinda like parse_entire_file, but the conditions are different)
statement_node *parse_block(parser_state *state) {
    parser_eat_newlines(state);

    statement_node *first_statement = NULL;
	if (state->current_token.kind != TOKEN_RBRACE) {
		first_statement = parse_statement(state);
        parser_eat_newlines(state);
    }

    statement_node *current_statement = first_statement;
	while (state->current_token.kind != TOKEN_RBRACE) {
        current_statement->next = parse_statement(state);
        parser_eat_newlines(state);
        current_statement = current_statement->next;
	}

    return first_statement;
}

statement_node* parse_entire_file(token_array *tokens, char *file_name, string file_contents) {
	parser_state state = {
        .tokens = tokens,
        .pos = 0,
        .current_token = token_array_get(tokens, 0),
        .file_name = file_name,
        .file_contents = file_contents,
    };

    parser_eat_newlines(&state);

    statement_node *first_statement = NULL;
	if (state.pos < state.tokens->length) {
		first_statement = parse_statement(&state);
    }

    statement_node *current_statement = first_statement;
	while (state.pos < state.tokens->length) {
		current_statement->next = parse_statement(&state);
		current_statement = current_statement->next;
	}
	return first_statement;
}

statement_node parse_declaration_or_assignment(parser_state *state) {
    if (state->current_token.kind != TOKEN_IDENTIFIER) {
        panic("expected lhs of statement to be identifier, got=%d", state->current_token.kind);
    }
    string lhs = state->current_token.value.identifier;
    parser_advance(state);
    if (state->current_token.kind == TOKEN_COLON) {
        parser_advance(state);
        if (state->current_token.kind != TOKEN_IDENTIFIER) {
            panic("expected declaration data type to be identifier, got=%d", state->current_token.kind);
        }
        string data_type = state->current_token.value.identifier;
        parser_advance(state);
        if (state->current_token.kind != TOKEN_ASSIGN) {
            parser_error(state, "Expected '=' in declaration, got %T", state->current_token);
        }
        parser_advance(state);
        return (statement_node){
            .kind = DECLARATION,
            .value.declaration = (declaration_node){
                .identifier = lhs,
                .data_type = data_type,
                .expression = parse_expression(state, MIN_PREC),
            },
        };
    } else if (state->current_token.kind == TOKEN_ASSIGN) {
        parser_advance(state);
        return (statement_node){
            .kind = ASSIGNMENT,
            .value.assignment = (assignment_node){
                .lhs = lhs,
                .expression = parse_expression(state, MIN_PREC),
            },
        };
    } else {
        parser_error(state, "Expected ':' or '=' in statement, got %T", state->current_token);
    }
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
        case TOKEN_BREAK:
            stmt->kind = BREAK;
            parser_advance(state);
            break;
        case TOKEN_CONTINUE:
            stmt->kind = CONTINUE;
            parser_advance(state);
            break;
        case TOKEN_IDENTIFIER:
            *stmt = parse_declaration_or_assignment(state);
            break;
        case TOKEN_WHILE:
            stmt->kind = WHILE_LOOP;
            parser_advance(state);
            stmt->value.while_loop.condition = parse_expression(state, MIN_PREC);
            if (state->current_token.kind != TOKEN_LBRACE) {
                parser_error(state, "Expected '{' after while-loop condition");
            }
            parser_advance(state);
            stmt->value.while_loop.body = parse_block(state);
            if (state->current_token.kind != TOKEN_RBRACE) {
                parser_error(state, "Expected '}' after while-loop body");
            }
            parser_advance(state);
            break;
		case TOKEN_IF:
            stmt->kind = CONDITIONAL;
            parser_advance(state);
			stmt->value.conditional.condition = parse_expression(state, MIN_PREC);
            if (state->current_token.kind != TOKEN_LBRACE) {
                parser_error(state, "Expected '{' after if-statement condition", state->current_token);
            }
            parser_advance(state);
			stmt->value.conditional.body = parse_block(state);
            if (state->current_token.kind != TOKEN_RBRACE) {
                parser_error(state, "Expected '}' after if-statement body", state->current_token);
            }
            parser_advance(state);
            if (state->current_token.kind != TOKEN_END_OF_LINE && state->current_token.kind != TOKEN_ELSE) {
                parser_error(state, "Expected new line or else before next statement");
            }
            parser_eat_newlines(state);
            // capture any potential elses since they shouldnt count as their own statement
            if (state->current_token.kind == TOKEN_ELSE) {
                parser_advance(state);
                // else {
                if (state->current_token.kind == TOKEN_LBRACE) {
                    parser_advance(state);
                    stmt->value.conditional.alt_body = parse_block(state);
                    if (state->current_token.kind != TOKEN_RBRACE) {
                        parser_error(state, "Expected '}' after else body", state->current_token);
                    }
                    parser_advance(state);
                }
                // else if
                else if (state->current_token.kind == TOKEN_IF) {
                    stmt->value.conditional.alt_body = parse_statement(state);
                }
                else {
                    parser_error(state, "Unexpected syntax after else keyword: '%T'", state->current_token);
                }
            }
            parser_eat_newlines(state);
            return stmt;
        case TOKEN_ELSE:
            parser_error(state, "Unrecognized syntax: else statement with no preceding if statement.");
        default:
            parser_error(state, "Expected statement, got %T", state->current_token);
    }
    if (state->current_token.kind != TOKEN_END_OF_LINE) {
        parser_error(state, "Unexpected token %T after end of statement", state->current_token);
    }
    parser_eat_newlines(state);
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
	else if (state->current_token.kind == TOKEN_TRUE) {
		leaf->kind = BOOL;
		leaf->value.bool = true;
	}
	else if (state->current_token.kind == TOKEN_FALSE) {
		leaf->kind = BOOL;
		leaf->value.bool = false;
	}
    else if (state->current_token.kind == TOKEN_FUNCTION) {
        leaf->kind = INLINE_FUNCTION;
        parser_advance(state);
        // expect left parenthesis
        if (state->current_token.kind != TOKEN_LPAREN) {
            parser_error(state, "Expected '(' after 'func' keyword");
        }
        parser_advance(state);
        // expect arguments here
        // expect right parenthesis
        if (state->current_token.kind != TOKEN_RPAREN) {
            parser_error(state, "Expected ')' after function arguments");
        }
        parser_advance(state);
        // expect left brace
        if (state->current_token.kind != TOKEN_LBRACE) {
            parser_error(state, "Expected '{' after function declaration");
        }
        parser_advance(state);
        // expect block of code
        leaf->value.function_declaration.body = parse_block(state);
        if (state->current_token.kind != TOKEN_RBRACE) {
            parser_error(state, "Expected '}' after function body");
        }
    }
	else {
        parser_error(state, "Expected expression, got %T", state->current_token);
	}
    parser_advance(state);
	return leaf;
}

int sprint_token(char *buf, token t) {
    int written = 0;
	switch (t.kind) {
		case TOKEN_ASSIGN: // fall through
		case TOKEN_PLUS: // fall through
        case TOKEN_MINUS: // fall through
        case TOKEN_STAR: // fall through 
        case TOKEN_SLASH: // fall through
        case TOKEN_DOT: // fall through
		case TOKEN_LESS_THAN: // fall through
		case TOKEN_GREATER_THAN: // fall through
        case TOKEN_LPAREN: // fall through
        case TOKEN_RPAREN: // fall through
		case TOKEN_LBRACE: // fall through
		case TOKEN_RBRACE:
			return sprintf(buf, "%c", t.kind);
        case TOKEN_END_OF_LINE:
			return sprintf(buf, "(newline)");
		case TOKEN_IDENTIFIER:
			written += sprintf(buf + written, "'");
			written += sprint_string(buf + written, t.value.identifier);
			written += sprintf(buf + written, "'");
			break;
		case TOKEN_INTEGER:
			return sprintf(buf, "%ld", t.value.integer);
        case TOKEN_RETURN:
            return sprintf(buf, "return");
		case TOKEN_LESS_THAN_OR_EQUAL_TO:
            return sprintf(buf, "<=");
		case TOKEN_GREATER_THAN_OR_EQUAL_TO:
            return sprintf(buf, ">=");
		case TOKEN_EQUAL_TO:
            return sprintf(buf, "==");
		case TOKEN_NOT_EQUAL_TO:
            return sprintf(buf, "!=");
        case TOKEN_FUNCTION:
            return sprintf(buf, "func");
        case TOKEN_IF:
            return sprintf(buf, "if");
        case TOKEN_ELSE:
            return sprintf(buf, "else");
		default:
			panic("print_token: unknown token kind=%d", t.kind);
	}
    return written;
}

bool is_operator(token t) {
	switch(t.kind) {
		case TOKEN_PLUS: // fall through
		case TOKEN_MINUS: // fall through
		case TOKEN_STAR: // fall through
		case TOKEN_SLASH: // fall through
		case TOKEN_DOT:
			return true;
		case TOKEN_LESS_THAN: // fall through
		case TOKEN_LESS_THAN_OR_EQUAL_TO: // fall through
		case TOKEN_GREATER_THAN: // fall through
		case TOKEN_GREATER_THAN_OR_EQUAL_TO: // fall through
		case TOKEN_EQUAL_TO: // fall through
		case TOKEN_NOT_EQUAL_TO:
            return true;
		default:
			return false;
	}
}
