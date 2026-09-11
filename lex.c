typedef struct token {
    enum {
        // For single-character tokens, just use their ascii value (that way
        // they can be printed directly):
        TOKEN_ASSIGN = '=',
        TOKEN_COLON = ':',
        TOKEN_PLUS = '+',
        TOKEN_MINUS = '-',
        TOKEN_STAR = '*',
        TOKEN_SLASH = '/',
		TOKEN_ZERO = '0',
		TOKEN_ONE = '1',
		TOKEN_TWO = '2',
		TOKEN_THREE = '3',
		TOKEN_FOUR = '4',
		TOKEN_FIVE = '5',
		TOKEN_SIX = '6',
		TOKEN_SEVEN = '7',
		TOKEN_EIGHT = '8',
		TOKEN_NINE = '9',
		TOKEN_L_PARENTHESIS = '(',
		TOKEN_R_PARENTHESIS = ')',
        TOKEN_END_OF_LINE = '\n',
        // multicharacter or non-ascii tokens start above the ascii range:
        TOKEN_IDENTIFIER = 128,
    } kind;
    union {
        string identifier; // Only used for identifiers
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

void token_array_append(token_array *arr, token tok) {
    if (arr->length + 1 > arr->capacity) {
        arr->capacity = arr->capacity * 2 + 1;
        void *new = xalloc(arr->capacity);
        memcpy(new, arr->buf, arr->length * sizeof(token));
        xfree(arr->buf);
        arr->buf = new;
    }

    arr->buf[arr->length + 1] = tok;

    arr->length += 1;
}

bool is_keyword_character(char ch) {
    return (ch >= 'a' && ch <= 'z') ||
        (ch >= 'A' && ch <= 'Z') ||
        ch == '_';
}

// 'i' is an outparam
token lex_identifier_or_keyword(string input, uint64_t *i) {
    string identifier = (string){
        .buf = input.buf + *i,
        .length = 0,
    };
    while (*i < input.length && is_keyword_character(input.buf[*i])) {
        *i += 1;
    }
    return (token){
        .kind = TOKEN_IDENTIFIER,
        .value.identifier = identifier,
    };
}

token_array lex_entire_file(string input) {
    token_array tokens = token_array_create();
    for (uint64_t i = 0; i < input.length; i++) {
        token tok = { 0 };
        char ch = input.buf[i];
        switch (ch) {
            // All the single character tokens can be converted directly:
            case TOKEN_ASSIGN: // fall through
            case TOKEN_COLON: // fall through
            case TOKEN_PLUS: // fall through
            case TOKEN_MINUS: // fall through
            case TOKEN_STAR: // fall through
            case TOKEN_SLASH: // fall through
			case TOKEN_ZERO:
			case TOKEN_ONE:
			case TOKEN_TWO:
			case TOKEN_THREE:
			case TOKEN_FOUR:
			case TOKEN_FIVE:
			case TOKEN_SIX:
			case TOKEN_SEVEN:
			case TOKEN_EIGHT:
			case TOKEN_NINE:
			case TOKEN_L_PARENTHESIS:
			case TOKEN_R_PARENTHESIS:
            case TOKEN_END_OF_LINE:
                tok.kind = ch;
                break;
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
            case TOKEN_END_OF_LINE:
                printf("Token (end of line)\n");
                break;
            default:
                printf("Token kind='%c'\n", tok.kind);
                break;
        }
    }
}
