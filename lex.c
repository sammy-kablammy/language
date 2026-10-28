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

// TODO bounds check on 'destination'
//
// Returns how many characters it wrote 
uint32_t sprint_token(char *destination, token tok) {
    switch (tok.kind) {
        case TOKEN_PLUS:
            *destination = TOKEN_PLUS;
            return 1;
            break;
        default:
            panic("This is not the real implementation %d", tok.kind);
    }
}

// TODO Think about what happens if our custom codes are expanded to
// backslashes or percents, as printf would almost certainly break.

// For now this relies on sprintf for most conversion codes, but this can be
// changed
//
// fmt must be non null
void compiler_printf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    uint32_t fmt_pos = 0;
    uint32_t buf_pos = 0;
    while (fmt[fmt_pos] != '\0') {
        if (fmt[fmt_pos] == '%') {
            fmt_pos += 1;
            char conversion_code = fmt[fmt_pos];
            if (conversion_code == 'T') {
                token tok = va_arg(args, token);
                buf_pos += sprint_token(buf + buf_pos, tok);
            } else if (conversion_code == 'S') {
                string str = va_arg(args, string);
                buf_pos += sprint_string(buf + buf_pos, str);
            } else if (conversion_code == '%') {
                buf[buf_pos] = '%';
                buf_pos += 1;
            } else if (conversion_code == '\0') {
                panic("Expected conversion code, but reached end of format string '%s'", fmt);
            } else if (conversion_code == 'd') {
                // For now we rely on libc's sprintf for most conversion codes
                buf_pos += sprintf(buf + buf_pos, "%d", va_arg(args, int));
            } else {
                panic("Unrecognized conversion code '%c'", conversion_code);
            }
            fmt_pos += 1;
            continue;
        } else {
            buf[buf_pos] = fmt[fmt_pos];
            buf_pos += 1;
            fmt_pos += 1;
            continue;
        }
    }

    // TODO bounds check
    buf[buf_pos] = '\0';

    fprintf(stdout, "%s", buf);

    va_end(args);
}

void lex_error(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
}

void debug_print_tokens(token_array tokens) {
    for (uint64_t i = 0; i < tokens.length; i++) {
        token tok = tokens.buf[i];

        switch (tok.kind) {
            case TOKEN_IDENTIFIER:
                printf("Token kind=identifier value='");
                print_string(tok.value.identifier);
                printf("'\n");

                lex_error("Token kind=identifier value='%T'\n", tok.value.identifier);
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
