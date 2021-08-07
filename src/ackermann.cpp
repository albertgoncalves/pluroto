#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// NOTE: See `https://www.geeksforgeeks.org/ackermanns-function-using-dynamic-programming/`.

typedef uint32_t u32;

typedef int32_t i32;

#define M 3
#define N 13

i32 main() {
    u32 table[M + 1][N + 1] = {};
    for (u32 j = 0; j <= N; ++j) {
        table[0][j] = j + 1;
    }
    for (u32 i = 1; i <= M; ++i) {
        table[i][0] = table[i - 1][1];
        for (u32 j = 1; j <= N; ++j) {
            u32 a = i - 1;
            u32 b = table[i][j - 1];
            if (a == 0) {
                table[i][j] = b + 1;
            } else if (b <= N) {
                table[i][j] = table[a][b];
            } else {
                table[i][j] = table[a][N] + ((b - N) * a);
            }
        }
    }
    for (u32 i = 0; i <= M; ++i) {
        for (u32 j = 0; j <= N; ++j) {
            printf(" %5u", table[i][j]);
        }
        printf("\n");
    }
    return EXIT_SUCCESS;
}
