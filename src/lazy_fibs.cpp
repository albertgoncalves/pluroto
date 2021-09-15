#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#define CAP_LISTS  (1 << 6)
#define CAP_THUNKS (1 << 6)
#define CAP_ARGS   (1 << 6)

typedef size_t usize;

typedef int32_t i32;
typedef int64_t i64;

#define null nullptr

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

typedef struct List   List;
typedef struct Thunk  Thunk;
typedef struct Memory Memory;

struct List {
    Thunk* thunk;
    i64    value;
};

typedef List* (*Func)(void*);

struct ThunkDefer {
    Func  func;
    void* payload;
};

union ThunkBody {
    ThunkDefer as_defer;
    List*      as_list;
};

struct Thunk {
    ThunkBody body;
    bool      cached;
};

struct Args {
    Memory* memory;
    List*   lists[2];
};

template <typename T, usize N>
struct Buffer {
    T     items[N];
    usize len;
};

struct Memory {
    Buffer<List, CAP_LISTS>   lists;
    Buffer<Thunk, CAP_THUNKS> thunks;
    Buffer<Args, CAP_ARGS>    args;
};

static List* FIBS = null;

template <typename T, usize N>
static T* alloc(Buffer<T, N>* buffer) {
    EXIT_IF(N <= buffer->len);
    return &buffer->items[buffer->len++];
}

static List* force(Thunk* thunk) {
    if (thunk->cached) {
        return thunk->body.as_list;
    }
    List* list = thunk->body.as_defer.func(thunk->body.as_defer.payload);
    thunk->cached = true;
    thunk->body.as_list = list;
    return list;
}

static Thunk* defer(Memory* memory, Func func, void* payload) {
    Thunk* thunk = alloc(&memory->thunks);
    thunk->cached = false;
    thunk->body.as_defer.func = func;
    thunk->body.as_defer.payload = payload;
    return thunk;
}

static i64 head(List* list) {
    return list->value;
}

static List* tail(List* list) {
    return force(list->thunk);
}

List* zip_sum(Memory*, List*, List*);

static List* f0(void* payload) {
    Args* args = reinterpret_cast<Args*>(payload);
    return zip_sum(args->memory, tail(args->lists[0]), tail(args->lists[1]));
}

List* zip_sum(Memory* memory, List* a, List* b) {
    List* list = alloc(&memory->lists);
    list->value = head(a) + head(b);
    Args* args = alloc(&memory->args);
    args->memory = memory;
    args->lists[0] = a;
    args->lists[1] = b;
    list->thunk = defer(memory, f0, reinterpret_cast<void*>(args));
    return list;
}

static List* drop(i32 n, List* list) {
    for (; 0 < n; --n) {
        list = tail(list);
    }
    return list;
}

static List* f1(void* payload) {
    return zip_sum(reinterpret_cast<Memory*>(payload), FIBS, tail(FIBS));
}

static List* f2(void* payload) {
    Memory* memory = reinterpret_cast<Memory*>(payload);
    List*   list = alloc(&memory->lists);
    list->value = 1;
    list->thunk = defer(memory, f1, payload);
    return list;
}

i32 main() {
    printf("sizeof(List)       : %zu\n"
           "sizeof(Func)       : %zu\n"
           "sizeof(ThunkDefer) : %zu\n"
           "sizeof(ThunkBody)  : %zu\n"
           "sizeof(Thunk)      : %zu\n"
           "sizeof(Args)       : %zu\n"
           "sizeof(Memory)     : %zu\n"
           "\n",
           sizeof(List),
           sizeof(Func),
           sizeof(ThunkDefer),
           sizeof(ThunkBody),
           sizeof(Thunk),
           sizeof(Args),
           sizeof(Memory));
    Memory* memory = reinterpret_cast<Memory*>(calloc(1, sizeof(Memory)));
    {
        FIBS = alloc(&memory->lists);
        FIBS->value = 0;
        FIBS->thunk = defer(memory, f2, reinterpret_cast<void*>(memory));
        i64 x = head(drop(50, FIBS));
        printf("%ld\n", x);
        EXIT_IF(x != 12586269025);
    }
    free(memory);
    return EXIT_SUCCESS;
}
