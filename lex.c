typedef struct token {
    enum {
        // This is never set, but it's convenient while debugging to have the
        // zero value represent something:
        TOKEN_INVALID = 0,
        // For single-character tokens, just use their ascii value (that way
        // they can be printed directly):
        TOKEN_ASSIGN = '=',
        TOKEN_PLUS = '+',
        TOKEN_MINUS = '-',
        TOKEN_STAR = '*',
        TOKEN_SLASH = '/',
        TOKEN_COLON = ':',
        TOKEN_DOT = '.',
        TOKEN_END_OF_LINE = '\n',
        // multicharacter or non-ascii tokens start above the ascii range:
        TOKEN_IDENTIFIER = 128,
        TOKEN_INTEGER,
        TOKEN_RETURN,
        TOKEN_TRUE,
        TOKEN_FALSE,
    } kind;
    union {
        string identifier; // Only used for identifiers
        uint64_t integer; // Only used for integers
    } value;
    // uint64_t position, // TODO this'll be handy for compiler errors
} token;

typedef struct token_array {
    token *buf;
    uint64_t length;
    uint64_t capacity;
} token_array;

token_array token_array_create() {
    const uint64_t initial_capacity = 10;
    return (token_array){
        .buf = xalloc(initial_capacity * sizeof(token)),
        .length = 0,
        .capacity = initial_capacity,
    };
}

// There's a lexer infinite loop somewhere here. Try to run the compiler on some
// of our C files
//
// perhaps memcpy?
void token_array_append(token_array *arr, token tok) {
    if (arr->length + 1 > arr->capacity) {
        arr->capacity = arr->capacity * 2 + 1;
        void *new = xalloc(arr->capacity * sizeof(token));
        memcpy(new, arr->buf, arr->length * sizeof(token));
        xfree(arr->buf);
        arr->buf = new;
    }
    arr->buf[arr->length] = tok;
    arr->length += 1;
}

bool is_keyword_character(char ch) {
    return (ch >= 'a' && ch <= 'z') ||
        (ch >= 'A' && ch <= 'Z') ||
		(ch >= '0' && ch <= '9') ||
        ch == '_';
}

bool is_integer_character(char c) {
    return c >= '0' && c <= '9';
}

// 'i' is an outparam
token lex_number(string input, uint64_t *i) {
    uint64_t integer = 0;
    while (*i < input.length && is_integer_character(input.buf[*i])) {
        integer *= 10;
        integer += input.buf[*i] - '0';
        *i += 1;
    }
    return (token){
        .kind = TOKEN_INTEGER,
        .value.integer = integer,
    };
}

// 'i' is an outparam
token lex_identifier_or_keyword(string input, uint64_t *i) {
    string identifier = (string){
        .buf = input.buf + *i,
        .length = 0,
    };
    while (*i < input.length && is_keyword_character(input.buf[*i])) {
        *i += 1;
        identifier.length += 1;
    }

    if (string_equals_cstring(identifier, "return")) {
        return (token){ .kind = TOKEN_RETURN };
    }
    if (string_equals_cstring(identifier, "true")) {
        return (token){ .kind = TOKEN_TRUE };
    }
    if (string_equals_cstring(identifier, "false")) {
        return (token){ .kind = TOKEN_FALSE };
    }
    // Other keywords go here

    return (token){
        .kind = TOKEN_IDENTIFIER,
        .value.identifier = identifier,
    };
}

token_array lex_entire_file(string input) {
    token_array tokens = token_array_create();
    uint64_t i = 0;
    while (i < input.length) {
        token tok = { 0 };
        char ch = input.buf[i];
        switch (ch) {
            // All the single character tokens can be converted directly:
            case TOKEN_ASSIGN: // fall through
            case TOKEN_PLUS: // fall through
            case TOKEN_MINUS: // fall through
            case TOKEN_STAR: // fall through
            case TOKEN_SLASH: // fall through
            case TOKEN_COLON: // fall through
            case TOKEN_DOT: // fall through
            case TOKEN_END_OF_LINE:
                i += 1;
                tok.kind = ch;
                break;
            case '0': // fall through
            case '1': // fall through
            case '2': // fall through
            case '3': // fall through
            case '4': // fall through
            case '5': // fall through
            case '6': // fall through
            case '7': // fall through
            case '8': // fall through
            case '9': // fall through
                tok = lex_number(input, &i);
                break;
			case ' ': // fall through
			case '\t':
				i += 1;
				continue;
            default:
                tok = lex_identifier_or_keyword(input, &i);
                break;
        }
        token_array_append(&tokens, tok);
    }
    return tokens;
}

void debug_print_tokens(token_array tokens) {
    for (uint64_t i = 0; i < tokens.length; i++) {
        token tok = tokens.buf[i];

        switch (tok.kind) {
            case TOKEN_IDENTIFIER:
                printf("Token kind=identifier value='");
                print_string(tok.value.identifier);
                printf("'\n");
                break;
            case TOKEN_INTEGER:
                printf("Token kind=integer value=%lu\n", tok.value.integer);
                break;
            case TOKEN_END_OF_LINE:
                printf("Token (end of line)\n");
                break;
            default:
                printf("Token kind='%c'\n", tok.kind);
                break;
        }
    }
}
