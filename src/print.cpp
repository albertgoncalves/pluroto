#include <stdint.h>
#include <string.h>
#include <unistd.h>

typedef size_t usize;

typedef int32_t i32;
typedef ssize_t isize;

#define STDOUT 1

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

#define EXIT_IF(condition)   \
    if (condition) {         \
        _exit(EXIT_FAILURE); \
    }

#define CAP_BUFFER (1 << 4)

static char  BUFFER[CAP_BUFFER];
static usize BUFFER_LEN = 0;

static void flush() {
    EXIT_IF(static_cast<isize>(BUFFER_LEN) != write(STDOUT, BUFFER, BUFFER_LEN));
    BUFFER_LEN = 0;
}

static void print(const char* chars, usize len) {
    for (;;) {
        if ((BUFFER_LEN + len) <= CAP_BUFFER) {
            memcpy(&BUFFER[BUFFER_LEN], chars, len);
            BUFFER_LEN += len;
            return;
        } else {
            usize slice = CAP_BUFFER - BUFFER_LEN;
            memcpy(&BUFFER[BUFFER_LEN], chars, slice);
            BUFFER_LEN += slice;
            flush();
            chars = &chars[slice];
            len -= slice;
        }
    }
}

static void println(const char* chars, usize len) {
    print(chars, len);
    print("\n", 1);
}

i32 main() {
    print("Lorem ipsum dolor sit amet, ", 28);
    print("consectetur ", 12);
    println("adipiscing elit.", 16);
    flush();
    return EXIT_SUCCESS;
}
