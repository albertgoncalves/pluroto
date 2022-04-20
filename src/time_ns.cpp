#include <stdint.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

typedef int32_t i32;

typedef uint32_t u32;
typedef uint64_t u64;

typedef struct timespec Time;

#define ERROR 1
#define OK    0

#define EXIT_IF(condition)           \
    if (condition) {                 \
        fprintf(stderr,              \
                "%s:%s:%d \"%s\"\n", \
                __FILE__,            \
                __func__,            \
                __LINE__,            \
                #condition);         \
        _exit(ERROR);                \
    }

static u64 get_time_monotonic() {
    Time time;
    EXIT_IF(clock_gettime(CLOCK_MONOTONIC, &time));
    return (static_cast<u64>(time.tv_sec) * 1000000000) +
           (static_cast<u64>(time.tv_nsec));
}

i32 main() {
    printf("%lu\n", get_time_monotonic());
    return OK;
}
