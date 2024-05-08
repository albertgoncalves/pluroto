#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// NOTE: See `https://adventofcode.com/2020/day/10`.

typedef uint64_t u64;
typedef int32_t  i32;

#define EXIT_IF(condition)                                                              \
    if (condition) {                                                                    \
        fprintf(stderr, "%s:%s:%d \"%s\"\n", __FILE__, __func__, __LINE__, #condition); \
        exit(EXIT_FAILURE);                                                             \
    }

static const u64 ITEMS[] = {
    0,   1,   2,   3,   4,   7,   8,   11,  12,  13,  14,  15,  18,  19,  20,  23,  24,  27,  28,
    29,  30,  31,  34,  37,  38,  39,  40,  41,  44,  45,  46,  47,  48,  51,  54,  55,  56,  59,
    60,  61,  62,  63,  66,  67,  68,  69,  70,  73,  74,  75,  76,  77,  80,  81,  82,  83,  84,
    87,  88,  89,  90,  91,  94,  95,  96,  97,  98,  101, 104, 105, 108, 109, 110, 111, 112, 115,
    118, 119, 120, 121, 124, 125, 126, 129, 130, 131, 132, 133, 136, 137, 140};

#define LEN (sizeof(ITEMS) / sizeof(ITEMS[0]))

static u64 MEMO[LEN];

i32 main() {
    MEMO[0] = 1;
    for (i32 i = 1; i < static_cast<i32>(LEN); ++i) {
        MEMO[i] = 0;
        for (i32 j = i - 1; (0 <= j) && ((ITEMS[i] - ITEMS[j]) <= 3); --j) {
            MEMO[i] += MEMO[j];
        }
    }
    EXIT_IF(MEMO[LEN - 1] != 3100448333024);
    printf("All good!\n");
    return EXIT_SUCCESS;
}
