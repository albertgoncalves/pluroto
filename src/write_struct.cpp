#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

typedef uint8_t u8;
typedef int32_t i32;
typedef double  f64;

#define null nullptr

typedef FILE        File;
typedef struct stat FileStat;

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

struct Payload {
    u8   arg0;
    i32  arg1;
    f64  arg2;
    char arg3[5];
};

#define EXIT_IF(condition)                                                              \
    if (condition) {                                                                    \
        fflush(stdout);                                                                 \
        fprintf(stderr, "%s:%s:%d \"%s\"\n", __FILE__, __func__, __LINE__, #condition); \
        _exit(EXIT_FAILURE);                                                            \
    }

i32 main(i32 n, const char** args) {
    EXIT_IF(n < 2);
    {
        Payload payload = {
            255,
            -1,
            10.0,
            "abcd",
        };
        i32 file = open(args[1], O_CREAT | O_RDWR | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        EXIT_IF(file < 0);
        // NOTE: See `https://gist.github.com/marcetcheverry/991042`.
        EXIT_IF(ftruncate(file, sizeof(Payload)));
        void* memory = mmap(null, sizeof(Payload), PROT_READ | PROT_WRITE, MAP_SHARED, file, 0);
        EXIT_IF(memory == MAP_FAILED);
        memcpy(memory, &payload, sizeof(Payload));
        msync(memory, sizeof(Payload), MS_SYNC);
        munmap(memory, sizeof(Payload));
        close(file);
    }
    {
        i32 file = open(args[1], O_RDONLY);
        EXIT_IF(file < 0);
        {
            FileStat stat;
            EXIT_IF(fstat(file, &stat) < 0)
            EXIT_IF(stat.st_size != sizeof(Payload));
        }
        Payload* memory =
            reinterpret_cast<Payload*>(mmap(null, sizeof(Payload), PROT_READ, MAP_SHARED, file, 0));
        EXIT_IF(memory == MAP_FAILED);
        printf("%hhu\n%d\n%.1f\n\"%s\"\n", memory->arg0, memory->arg1, memory->arg2, memory->arg3);
        munmap(memory, sizeof(Payload));
        close(file);
    }
    return EXIT_SUCCESS;
}
