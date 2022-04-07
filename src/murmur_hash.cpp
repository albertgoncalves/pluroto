#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef uint8_t  u8;
typedef uint32_t u32;
typedef size_t   usize;

typedef int32_t i32;

// NOTE: See `https://softwareengineering.stackexchange.com/questions/49550/which-hashing-algorithm-is-best-for-uniqueness-and-speed`.
// NOTE: See `https://en.wikipedia.org/wiki/MurmurHash`.

__attribute__((no_sanitize("integer"))) static u32 get_scramble(u32 k) {
    k *= 0xCC9E2D51;
    k = (k << 15) | (k >> 17);
    k *= 0x1B873593;
    return k;
}

__attribute__((no_sanitize("integer"))) static u32 get_murmur3(const u8* key,
                                                               u32       len,
                                                               u32 seed) {
    u32 h = seed;
    u32 k;
    u32 n = (len / sizeof(u32)) * sizeof(u32);
    for (u32 i = 0; i < n; i += sizeof(u32)) {
        memcpy(&k, &key[i], sizeof(u32));
        h ^= get_scramble(k);
        h = (h << 13) | (h >> 19);
        h = (h * 5) + 0xE6546B64;
    }
    k = 0;
    for (u32 i = len - n; i != 0; --i) {
        k <<= 8;
        k |= key[n + (i - 1)];
    }
    h ^= get_scramble(k);
    h ^= len;
    h ^= h >> 16;
    h *= 0x85EBCA6B;
    h ^= h >> 13;
    h *= 0xC2B2AE35;
    h ^= (h >> 16);
    return h;
}

#define GET_MURMUR3(literal, seed)                    \
    get_murmur3(reinterpret_cast<const u8*>(literal), \
                sizeof(literal) - 1,                  \
                seed)

#define SEED 1760947478

i32 main() {
    printf("%u\n%u\n",
           GET_MURMUR3("1234", SEED),
           GET_MURMUR3("foo bar baz", SEED));
    return EXIT_SUCCESS;
}
