#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef int32_t   i32;
typedef time_t    Epoch;
typedef struct tm Time;

#define CAP_CHARS 20

struct Memory {
    char buffer[CAP_CHARS];
};

i32 main() {
    Epoch epoch;
    time(&epoch);
    Time*   time = localtime(&epoch);
    Memory* memory = reinterpret_cast<Memory*>(calloc(1, sizeof(Memory)));
    snprintf(memory->buffer,
             CAP_CHARS,
             "%4d-%02d-%02d %02d:%02d:%02d",
             1900 + time->tm_year,
             time->tm_mon,
             time->tm_mday,
             time->tm_hour,
             time->tm_min,
             time->tm_sec);
    printf("%s\n", memory->buffer);
    free(memory);
    return EXIT_SUCCESS;
}
