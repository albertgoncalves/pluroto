#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// NOTE: See `https://www.geeksforgeeks.org/moser-de-bruijn-sequence/`.

typedef uint32_t u32;

typedef int32_t i32;

#define N 19

i32 main() {
    u32 table[N];
    table[0] = 0;
    table[1] = 1;
    for (u32 i = 2; i < N; i += 2) {
        table[i] = 4 * table[i / 2];
        if (N <= (i + 1)) {
            break;
        }
        table[i + 1] = table[i] + 1;
    }
    for (u32 i = 0; i < N; ++i) {
        printf(" %u", table[i]);
    }
    printf("\n");
    return EXIT_SUCCESS;
}
