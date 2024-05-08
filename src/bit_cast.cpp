#include <stdio.h>
#include <stdlib.h>

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;

#ifdef __LITTLE_ENDIAN__

static i8 read_i8(const i8* array, i32 i) {
    return array[i];
}

static i16 read_i16(const i8* array, i32 i) {
    i32 j = i << 1;
    return static_cast<i16>(array[j] | (array[j + 1] << 8));
}

static i32 read_i32(const i8* array, i32 i) {
    i32 j = i << 2;
    return array[j] | (array[j + 1] << 8) | (array[j + 2] << 16) | (array[j + 3] << 24);
}

#endif

i32 main() {
    i8 array[] = {
        0,  1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15,
        16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
    };
    printf("%hhd\n%hhd\n", array[10], read_i8(array, 10));
    printf("%hd\n%hd\n", read_i16(array, 1), *reinterpret_cast<i16*>(&array[1 << 1]));
    printf("%d\n%d\n", read_i32(array, 3), *reinterpret_cast<i32*>(&array[3 << 2]));
    return EXIT_SUCCESS;
}
