#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// NOTE: See `https://www.briansteffens.com/2017/06/19/split-buffers.html`.
// NOTE: See `https://en.wikipedia.org/wiki/Gap_buffer`.

typedef uint32_t u32;
typedef int32_t  i32;

#define CAP_CHARS 1 << 10

struct Memory {
    char  swap[2][CAP_CHARS];
    char* buffer;
    u32   gap_start;
    u32   post_start;
    u32   post_end;
};

#define EXIT_IF(condition)           \
    if (condition) {                 \
        fprintf(stderr,              \
                "%s:%s:%d \"%s\"\n", \
                __FILE__,            \
                __func__,            \
                __LINE__,            \
                #condition);         \
        exit(EXIT_FAILURE);          \
    }

#define GAP_WIDTH (1 << 3)

static void init(Memory* memory) {
    memory->buffer = &memory->swap[0][0];
    memory->gap_start = 0;
    memory->post_start = GAP_WIDTH;
    memory->post_end = memory->post_start;
}

static void grow(Memory* memory) {
    u32 start = memory->post_start + GAP_WIDTH;
    u32 len = memory->post_end - memory->post_start;
    u32 end = start + len;
    EXIT_IF(CAP_CHARS <= end);
    u32 from = 0;
    u32 to = 0;
    if (memory->buffer == &memory->swap[from][0]) {
        to = 1;
    } else {
        from = 1;
    }
    memcpy(&memory->swap[to][0], &memory->swap[from][0], memory->gap_start);
    memcpy(&memory->swap[to][start],
           &memory->swap[from][memory->post_start],
           len);
    memory->buffer = &memory->swap[to][0];
    memory->post_start = start;
    memory->post_end = end;
}

static void insert(Memory* memory, char x) {
    if (memory->gap_start == memory->post_start) {
        grow(memory);
    }
    memory->buffer[memory->gap_start++] = x;
}

static void delete_(Memory* memory) {
    if (memory->gap_start == 0) {
        return;
    }
    --memory->gap_start;
}

static void left(Memory* memory) {
    if (memory->gap_start == 0) {
        return;
    }
    memory->buffer[--memory->post_start] = memory->buffer[--memory->gap_start];
}

static void right(Memory* memory) {
    if (memory->gap_start == memory->post_end) {
        return;
    }
    memory->buffer[memory->gap_start++] = memory->buffer[memory->post_start++];
}

static void show(Memory* memory) {
    u32 i = 0;
    for (; i < memory->gap_start; ++i) {
        printf(" %c", memory->buffer[i]);
    }
    printf("[");
    for (; i < memory->post_start; ++i) {
        printf(" _");
    }
    printf("]");
    for (; i < memory->post_end; ++i) {
        printf(" %c", memory->buffer[i]);
    }
    printf("\n");
}

static void display(Memory* memory) {
    for (u32 i = 0; i < memory->gap_start; ++i) {
        printf("%c", memory->buffer[i]);
    }
    for (u32 i = memory->post_start; i < memory->post_end; ++i) {
        printf("%c", memory->buffer[i]);
    }
    printf("\n");
}

i32 main() {
    Memory* memory = reinterpret_cast<Memory*>(calloc(1, sizeof(Memory)));
    init(memory);
    {
        insert(memory, 'a');
        insert(memory, 'b');
        delete_(memory);
        insert(memory, 'c');
        left(memory);
        insert(memory, 'd');
        left(memory);
        insert(memory, 'e');
        right(memory);
        left(memory);
        left(memory);
        right(memory);
        right(memory);
        delete_(memory);
        insert(memory, 'f');
        insert(memory, 'g');
        insert(memory, 'h');
        insert(memory, 'i');
        left(memory);
        right(memory);
        delete_(memory);
        insert(memory, 'j');
        insert(memory, 'k');
        insert(memory, 'l');
        insert(memory, 'm');
        left(memory);
        left(memory);
        left(memory);
        left(memory);
        left(memory);
        left(memory);
    }
    {
        printf("\n");
        show(memory);
        display(memory);
    }
    free(memory);
    return EXIT_SUCCESS;
}
