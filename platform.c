// Platform layer: anything relying on OS-specific functionality. (Also libc
// wrappers and anything like that)

#include <assert.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct string {
    uint64_t length;
    char *buf;
} string;

typedef uint8_t bool;
const bool true = 1;
const bool false = 0;

string read_entire_file(const char *filename);

// TODO bounds check buf
uint32_t sprint_string(char *buf, string str) {
    uint64_t i = 0;
    while (i < str.length) {
        buf[i] = str.buf[i];
        i += 1;
    }
    return (uint32_t)i;
}

void print_string(string str) {
    for (uint64_t i = 0; i < str.length; i++) {
        printf("%c", str.buf[i]);
    }
}

bool cstring_equals_cstring(char *s1, char *s2) {
    // whatever
    return strcmp(s1, s2) == 0;
}

bool string_equals_cstring(string s1, const char *s2) {
    for (uint64_t i = 0; i < s1.length && s2[i] != '\0'; i++) {
        if (s1.buf[i] != s2[i]) {
            return false;
        }
    }
    return true;
}

__attribute__((noreturn)) void panic(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    fprintf(stderr, "[INTERNAL COMPILER ERROR]\n");
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");

    va_end(args);
    exit(67);
}

// zero-initializing, unfailable general purpose allocator
void *xalloc(size_t size) {
    void *buf = calloc(size, 1);
    if (buf == NULL) {
        panic("allocator failed");
    }
    return buf;
}

void xfree(void *buf) {
    free(buf);
}

// Custom vsprintf that supports compiler-specific format strings.
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
            case 's':
                buf_pos += sprintf(buf + buf_pos, "%s", va_arg(args, char *));
                break;
            case 'c':
                // (apparently %c expects an int, not a char)
                buf_pos += sprintf(buf + buf_pos, "%c", va_arg(args, int));
                break;
            case '%':
                buf[buf_pos] = '%';
                buf_pos += 1;
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

#if defined(__linux__)

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

string read_entire_file(const char *filename) {
    string ret = { 0 };

    struct stat st;
    int err = stat(filename, &st);
    if (err == -1) {
        panic("Unable to stat file '%s'\n", filename);
    }
    ret.length = st.st_size;

    int fd = open(filename, O_RDONLY);
    if (fd == -1) {
        panic("Unable to open file '%s'\n", filename);
    }

    ret.buf = mmap(NULL, ret.length, PROT_READ, MAP_SHARED, fd, 0);
    if (ret.buf == (void *)-1) {
        close(fd);
        panic("Unable to mmap file '%s'\n", filename);
    }

    return ret;
}

#else
#error "This operating system is not supported"
#endif
