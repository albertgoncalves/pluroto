#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef uint8_t u8;

typedef int8_t  i8;
typedef int32_t i32;

i32 main() {
    u8 x = 0x80;
    printf("%hhu %hhd\n", x, static_cast<i8>(x));
    return EXIT_SUCCESS;
}
