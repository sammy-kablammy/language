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
    uint64_t file_position;
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
void lex_number(string input, uint64_t *i, token *tok) {
    uint64_t integer = 0;
    while (*i < input.length && is_integer_character(input.buf[*i])) {
        integer *= 10;
        integer += input.buf[*i] - '0';
        *i += 1;
    }
    tok->kind = TOKEN_INTEGER;
    tok->value.integer = integer;
}

// 'i' is an outparam
void lex_identifier_or_keyword(string input, uint64_t *i, token *tok) {
    string identifier = (string){
        .buf = input.buf + *i,
        .length = 0,
    };
    while (*i < input.length && is_keyword_character(input.buf[*i])) {
        *i += 1;
        identifier.length += 1;
    }

    if (string_equals_cstring(identifier, "return")) {
        tok->kind = TOKEN_RETURN;
        return;
    }
    if (string_equals_cstring(identifier, "true")) {
        tok->kind = TOKEN_TRUE;
        return;
    }
    if (string_equals_cstring(identifier, "false")) {
        tok->kind = TOKEN_FALSE;
        return;
    }
    // Other keywords go here

    tok->kind = TOKEN_IDENTIFIER;
    tok->value.identifier = identifier;
}

token_array lex_entire_file(string input) {
    token_array tokens = token_array_create(input);
    uint64_t i = 0;
    while (i < input.length) {
        token tok = { 0 };
        tok.file_position = i;
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
                lex_number(input, &i, &tok);
                break;
			case ' ': // fall through
			case '\t':
				i += 1;
				continue;
            default:
                lex_identifier_or_keyword(input, &i, &tok);
                break;
        }
        token_array_append(&tokens, tok);
    }
    return tokens;
}

void lex_error(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    // fprintf(stdout, "%s", buf);
}



// Down here is the "reverse lexer"



// Trying to handle LF, CRLF, and CR but i haven't tested this that thoroughly

// Lines are 1-based
uint64_t token_to_line_number(string source_code, token tok) {
    if (tok.file_position >= source_code.length) {
        panic("Somehow a token thinks it's outside the bounds of a file");
    }

    uint64_t line_number = 1;
    for (uint64_t i = 0; i < source_code.length - 1 && i < tok.file_position; i++) {
        if (source_code.buf[i] == '\n') {
            // LF
            line_number += 1;
            continue;
        }
        if (source_code.buf[i] == '\r' && source_code.buf[i + 1] == '\n') {
            // CRLF
            line_number += 1;
            i += 1; // Don't double count the '\n' following '\r'
            continue;
        }
        if (source_code.buf[i] == '\r') {
            // CR
            line_number += 1;
            continue;
        }
    }
    return line_number;
}

string token_to_source_code_line(string source_code, token tok) {
    if (tok.file_position >= source_code.length) {
        panic("Somehow a token thinks it's outside the bounds of a file");
    }

    // If the token whose line we're looking up IS a newline, find the previous
    // nontrivial line instead.
    while (source_code.buf[tok.file_position] == '\n' || source_code.buf[tok.file_position] == '\r') {
        tok.file_position -= 1;
    }

    // leftward scan for line start
    uint64_t line_start = tok.file_position;
    while (line_start > 0 &&
            source_code.buf[line_start] != '\n' &&
            source_code.buf[line_start] != '\r') {
        line_start -= 1;
    }

    if (source_code.buf[line_start] == '\n' || source_code.buf[line_start] == '\r') {
        line_start += 1;
    }

    // rightward scan for line end
    uint64_t line_end = tok.file_position;
    while (source_code.buf[line_end] != '\r'
            && source_code.buf[line_end] != '\n'
            && line_end < source_code.length) {
        line_end += 1;
    }

    if (line_end <= line_start) {
        panic("token_to_source_code() this should never happen start=%d end=%d", line_start, line_end);
    }

    return (string) {
        .buf = source_code.buf + line_start,
        .length = line_end - line_start,
    };
}
