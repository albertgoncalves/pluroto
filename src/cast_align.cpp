#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef uint8_t u8;

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;

i32 main() {
    u8 buffer[4];
    {
        i32* view = reinterpret_cast<i32*>(buffer);
        view[0] = -1;
        printf("%d\n", view[0]);
    }
    for (u8 i = 0; i < 4; ++i) {
        printf("%hhu\n", buffer[i]);
    }
    return EXIT_SUCCESS;
}
