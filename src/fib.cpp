#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef uint8_t u8;

typedef int32_t i32;

#define N 14

i32 main() {
    u8 array[N];
    array[0] = 0;
    array[1] = 1;
    for (i32 i = 2; i < N; ++i) {
        array[i] = array[i - 2] + array[i - 1];
    }
    for (i32 i = 0; i < N; ++i) {
        printf(" %hhu", array[i]);
    }
    printf("\n");
    return EXIT_SUCCESS;
}
