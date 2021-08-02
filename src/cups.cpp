#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// NOTE: See `https://adventofcode.com/2020/day/23`.

typedef uint64_t u64;

typedef int32_t i32;

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

struct List {
    u64   value;
    List* next;
};

#define CAP 1000000

struct Memory {
    List lists[CAP];
};

const u64 ARRAY[] = {8, 7, 2, 4, 9, 5, 1, 3, 6};
const u64 INDEX[] = {6, 2, 7, 3, 5, 8, 1, 0, 4};

#define LEN (sizeof(ARRAY) / sizeof(ARRAY[0]))

static u64 get_index(u64 i) {
    if (i <= LEN) {
        return INDEX[i - 1];
    }
    return i - 1;
}

static u64 get_dst(u64 x) {
    return (((CAP - 2) + x) % CAP) + 1;
}

static List* update(Memory* memory, List* l0) {
    List* l1 = l0->next;
    List* l2 = l1->next;
    List* l3 = l2->next;
    u64   dst = get_dst(l0->value);
    while ((dst == l1->value) || (dst == l2->value) || (dst == l3->value)) {
        dst = get_dst(dst);
    }
    List* l4 = &memory->lists[get_index(dst)];
    List* l5 = l3->next;
    l3->next = l4->next;
    l4->next = l1;
    l0->next = l5;
    return l5;
}

i32 main() {
    Memory* memory = reinterpret_cast<Memory*>(calloc(1, sizeof(Memory)));
    EXIT_IF(!memory);
    for (u64 i = 0; i < LEN; ++i) {
        memory->lists[i] = {ARRAY[i], &memory->lists[(i + 1) % CAP]};
    }
    for (u64 i = LEN; i < CAP; ++i) {
        memory->lists[i] = {i + 1, &memory->lists[(i + 1) % CAP]};
    }
    {
        List* list = &memory->lists[0];
        for (u64 _ = 0; _ < 10000000; ++_) {
            list = update(memory, list);
        }
    }
    {
        List* l0 = memory->lists[get_index(1)].next;
        List* l1 = l0->next;
        EXIT_IF((l0->value * l1->value) != 170836011000);
    }
    free(memory);
    printf("All good!\n");
    return EXIT_SUCCESS;
}
