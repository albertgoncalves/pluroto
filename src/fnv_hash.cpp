#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// NOTE: See `https://softwareengineering.stackexchange.com/questions/49550/which-hashing-algorithm-is-best-for-uniqueness-and-speed`
// NOTE: See `https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function`

typedef uint8_t  u8;
typedef uint32_t u32;

typedef int32_t i32;

#define EXIT_IF(condition)           \
    if (condition) {                 \
        fprintf(stderr,              \
                "%s:%s:%d \"%s\"\n", \
                __FILE__,            \
                __func__,            \
                __LINE__,            \
                #condition);         \
        exit(EXIT_FAILURE);          \
    }

#define FNV_32_PRIME        16777619
#define FNV_32_OFFSET_BASIS 2166136261

static u32 fnv_1a_32(const u8* bytes, u32 len) {
    u32 hash = FNV_32_OFFSET_BASIS;
    for (u32 i = 0; i < len; ++i) {
        hash ^= bytes[i];
        hash *= FNV_32_PRIME;
    }
    return hash;
}

#define FNV_1A_32(literal) \
    fnv_1a_32(reinterpret_cast<const u8*>(literal), sizeof(literal) - 1)

i32 main() {
    // NOTE: See `https://github.com/sindresorhus/fnv1a`.
    EXIT_IF(FNV_1A_32("🦄🌈") != 2868248295);
    return EXIT_SUCCESS;
}
