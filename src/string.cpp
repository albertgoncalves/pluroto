#include <stdint.h>
#include <unistd.h>

typedef size_t usize;

typedef int32_t i32;
typedef ssize_t isize;

#define STDOUT 1

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

struct String {
    const char* chars;
    usize       len;
};

#define GET_STRING(literal)  \
    ((String){               \
        literal,             \
        sizeof(literal) - 1, \
    })

#define EXIT_IF(condition)       \
    {                            \
        if (condition) {         \
            _exit(EXIT_FAILURE); \
        }                        \
    }

static void print(String string) {
    EXIT_IF(static_cast<isize>(string.len) !=
            write(STDOUT, string.chars, string.len));
}

i32 main() {
    print(GET_STRING("Hello, world!\n"));
    return EXIT_SUCCESS;
}
