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
        TOKEN_LESS_THAN = '<',
        TOKEN_GREATER_THAN = '>',
        TOKEN_LPAREN = '(',
        TOKEN_RPAREN = ')',
        TOKEN_LBRACE = '{',
        TOKEN_RBRACE = '}',
        TOKEN_END_OF_LINE = '\n',
        // multicharacter or non-ascii tokens start above the ascii range:
        TOKEN_IDENTIFIER = 128,
        TOKEN_INTEGER,
        TOKEN_RETURN,
        TOKEN_TRUE,
        TOKEN_FALSE,
        TOKEN_LESS_THAN_OR_EQUAL_TO,
        TOKEN_GREATER_THAN_OR_EQUAL_TO,
        TOKEN_EQUAL_TO,
        TOKEN_NOT_EQUAL_TO,
        TOKEN_WHILE,
        TOKEN_FUNCTION,
    } kind;
    union {
        string identifier; // Only used for identifiers
        uint64_t integer; // Only used for integers
    } value;
    uint64_t file_position;
} token;

typedef array token_array;
void token_array_append(token_array *arr, token tok) {
    array_append(arr, &tok);
}
token token_array_get(token_array *arr, int i) {
    return *(token *)array_get(arr, i);
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
    if (string_equals_cstring(identifier, "while")) {
        tok->kind = TOKEN_WHILE;
        return;
    }
    if (string_equals_cstring(identifier, "func")) {
        tok->kind = TOKEN_FUNCTION;
        return;
    }
    // Other keywords go here

    if (identifier.length == 0) {
        panic("Expected identifier or keyword, got unrecognized character '%c'", input.buf[*i]);
    }
    tok->kind = TOKEN_IDENTIFIER;
    tok->value.identifier = identifier;
}

void skip_until(string input, uint64_t *i, char token) {
	while (input.buf[*i] != token) {
		if (*i >= input.length) {
			return;
		}
		*i += 1;
	}
}

void skip_spaces(string input, uint64_t *i) {
	while (input.buf[*i] == ' ' || input.buf[*i] == '\t') {
		if (*i >= input.length) {
			// realistically this can totally happen in a valid file, where they just have spaces at the end of their file.
			// optionally warn them to that they have whitespace at the end of the file. but is that our job as the compiler.. hm.
			// if we want to talk about all these toolchains that are produced and how annoying it is to have to use 3 different
			// applications to effectively develop, then by all means the compiler should take this responsibility.
			return;
		}
		*i += 1;
	}
}

token_array lex_entire_file(string input) {
    token_array tokens;
    array_init(&tokens, sizeof(token));

    uint64_t i = 0;
	int multiline_openers = 0;
    while (i < input.length) {
        token tok = { 0 };
        tok.file_position = i;
        char ch = input.buf[i];
        switch (ch) {
			case TOKEN_SLASH:
				multiline_openers = 0;
				i += 1;
				if (i >= input.length) {
					panic("EOF reached while attempting to parsing\n");
				}
				else { /* do nothing */ }
				
				if (input.buf[i] == TOKEN_SLASH) { // a single line comment
					skip_until(input, &i, TOKEN_END_OF_LINE);
					continue;
				}
				else if (input.buf[i] == TOKEN_STAR) { // a multi line comment
					multiline_openers += 1;
					i += 1;
					while (multiline_openers > 0) {
						if (i >= input.length) {
							panic("EOF reached while parsing multi-line comment.\n");
						}
						else { /* do nothing */ }
						
						if (input.buf[i] == TOKEN_SLASH) {
							i += 1;
							if (i >= input.length) {
								panic("EOF reached while parsing multi-line comment.\n");
							}
							else if (input.buf[i] == TOKEN_SLASH) { // a nested single line comment
								skip_until(input, &i, TOKEN_END_OF_LINE);
							}
							else if (input.buf[i] == TOKEN_STAR) { // a nested multi line comment
								multiline_openers += 1;
								i += 1;
							}
							else { /* do nothing */ }
						}
						else if (input.buf[i] == TOKEN_STAR) {
							i += 1;
							if (i >= input.length) {
								panic("EOF reached while parsing multi-line comment.\n");
							}
							else if (input.buf[i] == TOKEN_SLASH) { // end of a multi line comment
								multiline_openers -= 1;
								i += 1;
							}
							else { /* do nothing */ }
						}
						else { // normal part of comment parsed
							i += 1;
						}
					}
					continue;
				}
				else { // not a comment
					tok.kind = ch;
					break;
				}
			// All the single character tokens can be converted directly:
			case TOKEN_STAR: // fall through
            case TOKEN_PLUS: // fall through
            case TOKEN_MINUS: // fall through
            case TOKEN_COLON: // fall through
            case TOKEN_DOT: // fall through
            case TOKEN_LPAREN: // fall through
            case TOKEN_RPAREN: // fall through
            case TOKEN_LBRACE: // fall through
            case TOKEN_RBRACE: // fall through
            case TOKEN_END_OF_LINE:
                i += 1;
                tok.kind = ch;
                break;
            case '<':
                i += 1;
                if (i < input.length && input.buf[i] == '=') {
                    i += 1;
                    tok.kind = TOKEN_LESS_THAN_OR_EQUAL_TO;
                } else {
                    tok.kind = TOKEN_LESS_THAN;
                }
                break;
            case '>':
                i += 1;
                if (i < input.length && input.buf[i] == '=') {
                    i += 1;
                    tok.kind = TOKEN_GREATER_THAN_OR_EQUAL_TO;
                } else {
                    tok.kind = TOKEN_GREATER_THAN;
                }
                break;
            case '=':
                i += 1;
                if (i < input.length && input.buf[i] == '=') {
                    i += 1;
                    tok.kind = TOKEN_EQUAL_TO;
                } else {
                    tok.kind = TOKEN_ASSIGN;
                }
                break;
            case '!':
                i += 1;
                if (i < input.length && input.buf[i] == '=') {
                    i += 1;
                    tok.kind = TOKEN_NOT_EQUAL_TO;
                } else {
                    panic("! token hasn't been added yet");
                }
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
				skip_spaces(input, &i);
				continue;
            default:
                lex_identifier_or_keyword(input, &i, &tok);
                break;
        }
        token_array_append(&tokens, tok);
    }
    return tokens;
}

// note: lines are 1-based
uint64_t token_to_line_number(string source_code, token tok) {
    if (tok.file_position >= source_code.length) {
        panic("Somehow a token thinks it's outside the bounds of a file");
    }

    uint64_t line_number = 1;
    for (uint64_t i = 0; i < source_code.length - 1 && i < tok.file_position; i++) {
        if (source_code.buf[i] == '\r' && source_code.buf[i + 1] == '\n') {
            line_number += 1;
            i += 1; // don't double count the '\n' following the '\r'
            continue;
        }
        if (source_code.buf[i] == '\n') {
            line_number += 1;
            continue;
        }
        if (source_code.buf[i] == '\r') {
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

    uint64_t line_start = tok.file_position;
    while (line_start > 0 &&
            source_code.buf[line_start] != '\n' &&
            source_code.buf[line_start] != '\r') {
        line_start -= 1;
    }
    if (source_code.buf[line_start] == '\n' || source_code.buf[line_start] == '\r') {
        line_start += 1;
    }

    uint64_t line_end = tok.file_position;
    while (source_code.buf[line_end] != '\r'
            && source_code.buf[line_end] != '\n'
            && line_end < source_code.length) {
        line_end += 1;
    }

    if (line_end <= line_start) {
        panic("token_to_source_code_line: somehow bounds are backwards (%ld<=%ld)", line_end, line_start);
    }

    return (string) {
        .buf = source_code.buf + line_start,
        .length = line_end - line_start,
    };
}
