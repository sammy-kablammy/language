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
    uint64_t s2_length = strlen(s2);
    if (s1.length != s2_length) {
        return false;
    }
    for (uint64_t i = 0; i < s1.length && s2_length; i++) {
        if (s1.buf[i] != s2[i]) {
            return false;
        }
    }
    return true;
}

bool string_equals(string s1, string s2) {
    if (s1.length != s2.length) {
        return false;
    }
    for (uint64_t i = 0; i < s1.length && i < s2.length; i++) {
        if (s1.buf[i] != s2.buf[i]) {
            return false;
        }
    }
    return true;
}

string string_from_cstring(char *cstring) {
    int len = 0;
    while (cstring[len] != '\0') {
        len += 1;
    }
    return (string){
        .length = len,
        .buf = cstring,
    };
}

__attribute__((noreturn))
__attribute__((format(printf, 1, 2)))
void panic(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);

    fflush(stdout); // panic can sometimes beat stdout buffering

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
