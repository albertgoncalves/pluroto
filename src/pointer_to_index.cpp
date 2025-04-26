#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef uint16_t u16;

typedef int32_t i32;

i32 main() {
    u16 array[] = {
        0,
        1,
        2,
        3,
        4,
    };
    u16* a = &array[1];
    u16* b = &array[2];
    u16* c = &array[4];
    printf("%ld\n%ld\n%ld\n", a - array, b - array, c - array);
    return EXIT_SUCCESS;
}
