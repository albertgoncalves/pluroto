#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <cstring>

#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

typedef int32_t i32;
typedef int64_t i64;

typedef size_t usize;

typedef struct stat StatBuffer;

#define EXIT_WITH_IF(condition, message)                                \
    do {                                                                \
        if (condition) {                                                \
            fflush(stdout);                                             \
            fprintf(stderr, __FILE__ ":%d: %s\n", __LINE__, (message)); \
            _exit(1);                                                   \
        }                                                               \
    } while (0)

#define EXIT_IF(condition)       EXIT_WITH_IF(condition, #condition)
#define EXIT_ERRNO_IF(condition) EXIT_WITH_IF(condition, strerror(errno))

struct MmapFile {
    i32   file;
    void* address;
    usize len;

    MmapFile(const char* path) {
        file = open(path, O_RDONLY);
        EXIT_ERRNO_IF(file == -1);

        StatBuffer stat;
        EXIT_ERRNO_IF(fstat(file, &stat) != 0);
        len = static_cast<usize>(stat.st_size);
        EXIT_IF(len == 0);

        address = mmap(nullptr, len, PROT_READ, MAP_SHARED, file, 0);
        EXIT_ERRNO_IF(address == MAP_FAILED);
    }

    ~MmapFile() {
        EXIT_ERRNO_IF(munmap(address, len) != 0);
        EXIT_ERRNO_IF(close(file) != 0);
    }
};

i32 main() {
    printf("sizeof(MmapFile): %zu\n", sizeof(MmapFile));

    const char* path = "./out/mmap_file.txt";

    {
        const i32 file =
            open(path, O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
        EXIT_ERRNO_IF(file == -1);
        EXIT_ERRNO_IF(write(file, "Hello, world!", 13) == -1);
        EXIT_ERRNO_IF(close(file) != 0);
    }

    MmapFile mmap_file(path);
    printf("%.*s\n", static_cast<i32>(mmap_file.len), reinterpret_cast<char*>(mmap_file.address));

    return 0;
}
