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

string read_entire_file(const char *filename);

void print_string(string str) {
    for (uint64_t i = 0; i < str.length; i++) {
        printf("%c", str.buf[i]);
    }
}

void panic(const char *fmt, ...) {
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
