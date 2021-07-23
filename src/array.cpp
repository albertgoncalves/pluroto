#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef uint8_t u8;
typedef size_t  usize;

typedef int32_t i32;

template <typename T, usize N>
struct Array {
    T items[N];
};

static void show(u8 x) {
    printf("%hhu", x);
}

template <typename T, usize N>
static void show(Array<T, N>* array) {
    printf("[ ");
    for (usize i = 0; i < N; ++i) {
        show(array->items[i]);
        printf(" ");
    }
    printf("]\n");
}

i32 main() {
    Array<u8, 8> array = {};
    array.items[0] = 1;
    array.items[1] = 0;
    array.items[2] = 3;
    array.items[3] = 2;
    array.items[4] = 5;
    array.items[5] = 4;
    array.items[6] = 7;
    array.items[7] = 6;
    show(&array);
    return EXIT_SUCCESS;
}
