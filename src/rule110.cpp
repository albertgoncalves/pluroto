#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

typedef uint64_t u64;

typedef int32_t i32;

#define EXIT_SUCCESS 0

#define CAP_BUFFER (sizeof(u64) * 8ul)

static u64 get(u64 buffer, u64 i) {
    return (buffer >> i) & 1ul;
}

static void set(u64* buffer, u64 i) {
    *buffer |= 1ul << i;
}

static void clear(u64* buffer, u64 i) {
    *buffer &= ~(1ul << i);
}

i32 main() {
    u64 buffer = 0ul;
    set(&buffer, CAP_BUFFER - 2ul);
    for (u64 i = 0ul; i < (CAP_BUFFER - 2ul); ++i) {
        for (u64 j = 0ul; j < (CAP_BUFFER - 1ul); ++j) {
            fputc(" *"[get(buffer, j)], stdout);
        }
        fputc('\n', stdout);
        u64 bits = (get(buffer, 0ul) << 1ul) | get(buffer, 1ul);
        for (u64 j = 1ul; j < (CAP_BUFFER - 1ul); ++j) {
            bits = ((bits << 1ul) | get(buffer, j + 1ul)) & 7ul;
            if ((110ul >> bits) & 1ul) {
                set(&buffer, j);
            } else {
                clear(&buffer, j);
            }
        }
    }
    return EXIT_SUCCESS;
}
