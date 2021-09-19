#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/resource.h>
#include <unistd.h>

typedef size_t usize;

typedef int32_t i32;
typedef ssize_t isize;

typedef rlimit Limit;

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

template <typename T, usize N>
struct Buffer {
    T     items[N];
    usize len;
};

static void* alloc(usize size) {
    void* memory = sbrk(static_cast<isize>(size));
    if (memory == reinterpret_cast<void*>(-1)) {
        _exit(EXIT_FAILURE);
    }
    memset(memory, 0, size);
    return memory;
}

i32 main() {
    Limit limit = {};
    // NOTE: See `$ man 2 getrlimit`.
    getrlimit(RLIMIT_DATA, &limit);
    printf("%lu\n", limit.rlim_cur);
    Buffer<i32, 1 << 4>* memory = reinterpret_cast<Buffer<i32, 1 << 4>*>(
        alloc(sizeof(Buffer<i32, 1 << 4>)));
    (void)memory;
    return EXIT_SUCCESS;
}
