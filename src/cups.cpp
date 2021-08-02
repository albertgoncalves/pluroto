#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// NOTE: See `https://adventofcode.com/2020/day/23`.

typedef uint64_t u64;

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

#define CAP 1000000

struct Memory {
    u64 next[CAP + 1];
};

const u64 ARRAY[] = {8, 7, 2, 4, 9, 5, 1, 3, 6};

#define LEN (sizeof(ARRAY) / sizeof(ARRAY[0]))

i32 main() {
    Memory* memory = reinterpret_cast<Memory*>(calloc(1, sizeof(Memory)));
    EXIT_IF(!memory);
    {
        u64* next = memory->next;
        for (u64 i = 0; i < CAP; ++i) {
            next[i] = i + 1;
        }
        next[0] = ARRAY[0];
        next[CAP] = ARRAY[0];
        for (u64 i = 0; i < (LEN - 1); ++i) {
            next[ARRAY[i]] = ARRAY[i + 1];
        }
        next[ARRAY[LEN - 1]] = LEN + 1;
        {
            for (u64 i = 0; i < (LEN + 3); ++i) {
                printf(" %lu", next[i]);
            }
            printf("\n");
        }
        {
            u64 cur = 0;
            for (u64 i = 0; i <= 10000000; ++i) {
                cur = next[cur];
                u64 dst = ((CAP + cur - 2) % CAP) + 1;
                u64 n1 = next[cur];
                u64 n2 = next[n1];
                u64 n3 = next[n2];
                while ((dst == n1) || (dst == n2) || (dst == n3)) {
                    dst = (dst + CAP - 1) % CAP;
                }
                {
                    u64 t = next[n3];
                    next[n3] = next[dst];
                    next[dst] = next[cur];
                    next[cur] = t;
                }
            }
        }
        EXIT_IF((next[1] * next[next[1]]) != 170836011000);
    }
    free(memory);
    printf("All good!\n");
    return EXIT_SUCCESS;
}
