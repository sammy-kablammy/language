// Compiler error messages should go through this file.
// it centralizes our reliance on printf and other c standard lib functions

// Each phase of the compiler should have its own exit code, possibly multiple.
// This enables testing scripts to assert that a given program does NOT compile,
// and for a particular reason. For example a test could be like "Ensure that
// file.lang contains a type error that the compiler catches"



int sprint_token(char *buf, token t);



#define ANSI_BOLD "\x1b[1m"
#define ANSI_FAINT "\x1b[2m"
#define ANSI_ITALIC "\x1b[3m"
#define ANSI_UNDERLINE "\x1b[4m"
#define ANSI_STRIKETHROUGH "\x1b[9m"

#define ANSI_RED "\x1b[31m"
#define ANSI_GREEN "\x1b[32m"
#define ANSI_YELLOW "\x1b[33m"
#define ANSI_BLUE "\x1b[34m"
#define ANSI_MAGENTA "\x1b[35m"
#define ANSI_CYAN "\x1b[36m"

#define ANSI_RESET "\x1b[0m"

char error_buffer[100000];



// %S for struct strings (not C strings, those are %s)
// %T for token
//
// (Most printfs don't use these afaik)


// Custom vsprintf that supports compiler-specific format strings.
// Unfortunately, this also means we can't use the compiler's special printf
// attribute magic -- if the number of arguments doesn't match the format
// string, this will segfault.
//
// 'buf' should be really big.
//
// Return number of bytes written, excluding NUL. So writing "hey" returns 3.
//
// For now, fall back on libc's sprintf for non-custom formatting.
int compiler_vsprintf(char *buf, const char *fmt, va_list args) {
    if (fmt == NULL) {
        panic("compiler_vsprintf format string is NULL");
    }

    uint32_t fmt_pos = 0;
    uint32_t buf_pos = 0;
    while (fmt[fmt_pos] != '\0') {
        if (fmt[fmt_pos] != '%') {
            buf[buf_pos] = fmt[fmt_pos];
            buf_pos += 1;
            fmt_pos += 1;
            continue;
        }
        fmt_pos += 1;

        char conversion_code = fmt[fmt_pos];
        fmt_pos += 1;
        switch (conversion_code) {
            case 'T':
                buf_pos += sprint_token(buf + buf_pos, va_arg(args, token));
                break;
            case 'S':
                buf_pos += sprint_string(buf + buf_pos, va_arg(args, string));
                break;
            case 'd':
                buf_pos += sprintf(buf + buf_pos, "%d", va_arg(args, int));
                break;
            case 'u':
                buf_pos += sprintf(buf + buf_pos, "%u", va_arg(args, unsigned int));
                break;
            case 'f':
                buf_pos += sprintf(buf + buf_pos, "%f", va_arg(args, double));
                break;
            case 'x':
                buf_pos += sprintf(buf + buf_pos, "%x", va_arg(args, unsigned int));
                break;
            case 'p':
                buf_pos += sprintf(buf + buf_pos, "%p", va_arg(args, void *));
                break;
            case 's':
                buf_pos += sprintf(buf + buf_pos, "%s", va_arg(args, char *));
                break;
            case 'c':
                // (apparently %c expects an int, not a char)
                buf_pos += sprintf(buf + buf_pos, "%c", va_arg(args, int));
                break;
            case '%':
                buf_pos += sprintf(buf + buf_pos, "%%");
                break;
            case '\0':
                panic("Expected conversion code, but reached end of format string '%s'", fmt);
                break;
            default:
                panic("Unrecognized conversion code '%%%c' in format string '%s'", conversion_code, fmt);
                break;
        }
    }
    buf[buf_pos] = '\0';
    return buf_pos;
}

int compiler_sprintf(char *buf, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    int written = compiler_vsprintf(buf, fmt, args);
    va_end(args);
    return written;
}

int compiler_printf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char buf[10000] = { 0 };
    int written = compiler_vsprintf(buf, fmt, args);
    printf("%s", buf);
    va_end(args);
    return written;
}

int compiler_fprintf(FILE *file, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    char buf[10000] = { 0 };
    int written = compiler_vsprintf(buf, fmt, args);
    fprintf(file, "%s", buf);
    va_end(args);
    return written;
}

// This naming scheme mirrors libc, not sure which to use yet
#define cprintf compiler_printf
#define csprintf compiler_sprintf
#define cfprintf compiler_fprintf




typedef enum {
    GENERIC_COMPILER_ERROR,
    LEX_ERROR,
    PARSE_ERROR,
    TYPE_ERROR,
} error_category;

__attribute__((noreturn))
__attribute__((format(printf, 1, 2)))
void compiler_error(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    fprintf(stderr, "Parse error\n");
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");

    va_end(args);
    exit(67);
}
