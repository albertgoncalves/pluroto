#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef uint8_t u8;
typedef size_t  usize;

typedef int32_t i32;

static void show(u8 x) {
    printf("%hhu", x);
}

template <typename T, usize N> static void show(T array[N]) {
    printf("[ ");
    for (usize i = 0; i < N; ++i) {
        show(array[i]);
        printf(" ");
    }
    printf("]\n");
}

#define N 8

i32 main() {
    u8 array[N] = {};
    array[0] = 1;
    array[1] = 0;
    array[2] = 3;
    array[3] = 2;
    array[4] = 5;
    array[5] = 4;
    array[6] = 7;
    array[7] = 6;
    show<u8, N>(array);
    return EXIT_SUCCESS;
}
