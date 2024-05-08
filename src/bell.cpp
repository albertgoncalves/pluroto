#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// NOTE: See `https://www.geeksforgeeks.org/bell-numbers-number-of-ways-to-partition-a-set/`.
// NOTE: See `https://oeis.org/A000110`.

typedef uint64_t u64;

typedef int32_t i32;

#define EXIT_IF(condition)                                                              \
    if (condition) {                                                                    \
        fprintf(stderr, "%s:%s:%d \"%s\"\n", __FILE__, __func__, __LINE__, #condition); \
        exit(EXIT_FAILURE);                                                             \
    }

#define N (1 << 5)

static u64 MEMO[N][N];

static u64 _bell_rec_memo(u64 i, u64 j) {
    if (MEMO[i][j] != 0) {
        return MEMO[i][j];
    }
    u64 result;
    if ((i == 0) && (j == 0)) {
        result = 1;
    } else if (j == 0) {
        result = _bell_rec_memo(i - 1, i - 1);
    } else {
        result = _bell_rec_memo(i - 1, j - 1) + _bell_rec_memo(i, j - 1);
    }
    MEMO[i][j] = result;
    return result;
}

static u64 bell_rec_memo(u64 n) {
    EXIT_IF(N <= (n + 1));
    memset(MEMO, 0, sizeof(MEMO));
    return _bell_rec_memo(n, 0);
}

static u64 TABLE[N][N];

static u64 bell_table(u64 n) {
    EXIT_IF(N <= (n + 1));
    TABLE[0][0] = 1;
    for (u64 i = 1; i <= n; ++i) {
        TABLE[i][0] = TABLE[i - 1][i - 1];
        for (u64 j = 1; j <= i; ++j) {
            TABLE[i][j] = TABLE[i - 1][j - 1] + TABLE[i][j - 1];
        }
    }
    return TABLE[n][0];
}

i32 main() {
    printf("bell_rec_memo(20) : %lu\n", bell_rec_memo(20));
    printf("bell_table(20)    : %lu\n", bell_table(20));
    return EXIT_SUCCESS;
}
