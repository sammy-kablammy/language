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

uint64_t sprint_string(char *buf, string str) {
    uint64_t i = 0;
    while (i < str.length) {
        buf[i] = str.buf[i];
        i += 1;
    }
    return i;
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

typedef enum {
    INTERNAL_COMPILER_ERROR = 1,
    GENERIC_COMPILER_ERROR,
    LEX_ERROR,
    PARSE_ERROR,
    TYPECHECK_ERROR,
} exit_code;

__attribute__((noreturn)) void panic(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    fprintf(stderr, "[INTERNAL COMPILER ERROR]\n");
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");

    va_end(args);
    exit(INTERNAL_COMPILER_ERROR);
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
