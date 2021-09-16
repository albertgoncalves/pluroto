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

template <typename>
struct Thunk;

template <typename T>
struct List {
    Thunk<T>* thunk;
    T         value;
};

template <typename T>
struct ThunkDefer {
    List<T>* (*func)(void*);
    void* payload;
};

template <typename T>
union ThunkBody {
    ThunkDefer<T> as_defer;
    List<T>*      as_list;
};

template <typename T>
struct Thunk {
    ThunkBody<T> body;
    bool         cached;
};

template <typename>
struct Memory;

template <typename T>
struct Args {
    Memory<T>* memory;
    List<T>*   lists[2];
};

template <typename T, usize N>
struct Buffer {
    T     items[N];
    usize len;
};

template <typename T>
struct Memory {
    Buffer<List<T>, CAP_LISTS>   lists;
    Buffer<Thunk<T>, CAP_THUNKS> thunks;
    Buffer<Args<T>, CAP_ARGS>    args;
};

static List<i64>* FIBS = null;

template <typename T, usize N>
static T* alloc(Buffer<T, N>* buffer) {
    EXIT_IF(N <= buffer->len);
    return &buffer->items[buffer->len++];
}

template <typename T>
static List<T>* force(Thunk<T>* thunk) {
    if (thunk->cached) {
        return thunk->body.as_list;
    }
    List<T>* list = thunk->body.as_defer.func(thunk->body.as_defer.payload);
    thunk->cached = true;
    thunk->body.as_list = list;
    return list;
}

template <typename T>
static Thunk<T>* defer(Memory<T>* memory,
                       List<T>* (*func)(void*),
                       void* payload) {
    Thunk<T>* thunk = alloc(&memory->thunks);
    thunk->cached = false;
    thunk->body.as_defer.func = func;
    thunk->body.as_defer.payload = payload;
    return thunk;
}

template <typename T>
static T head(List<T>* list) {
    return list->value;
}

template <typename T>
static List<T>* tail(List<T>* list) {
    return force(list->thunk);
}

template <typename T>
List<T>* zip_sum(Memory<T>*, List<T>*, List<T>*);

template <typename T>
static List<T>* f0(void* payload) {
    Args<T>* args = reinterpret_cast<Args<T>*>(payload);
    return zip_sum(args->memory, tail(args->lists[0]), tail(args->lists[1]));
}

template <typename T>
List<T>* zip_sum(Memory<T>* memory, List<T>* a, List<T>* b) {
    List<T>* list = alloc(&memory->lists);
    list->value = head(a) + head(b);
    Args<T>* args = alloc(&memory->args);
    args->memory = memory;
    args->lists[0] = a;
    args->lists[1] = b;
    list->thunk = defer(memory, f0, reinterpret_cast<void*>(args));
    return list;
}

template <typename T>
static List<T>* drop(i32 n, List<T>* list) {
    for (; 0 < n; --n) {
        list = tail(list);
    }
    return list;
}

template <typename T>
static List<T>* f1(void* payload) {
    return zip_sum(reinterpret_cast<Memory<T>*>(payload), FIBS, tail(FIBS));
}

template <typename T>
static List<T>* f2(void* payload) {
    Memory<T>* memory = reinterpret_cast<Memory<T>*>(payload);
    List<T>*   list = alloc(&memory->lists);
    list->value = 1;
    list->thunk = defer(memory, f1, payload);
    return list;
}

i32 main() {
    printf("sizeof(List<i64>)       : %zu\n"
           "sizeof(ThunkDefer<i64>) : %zu\n"
           "sizeof(ThunkBody<i64>)  : %zu\n"
           "sizeof(Thunk<i64>)      : %zu\n"
           "sizeof(Args<i64>)       : %zu\n"
           "sizeof(Memory<i64>)     : %zu\n"
           "\n",
           sizeof(List<i64>),
           sizeof(ThunkDefer<i64>),
           sizeof(ThunkBody<i64>),
           sizeof(Thunk<i64>),
           sizeof(Args<i64>),
           sizeof(Memory<i64>));
    Memory<i64>* memory =
        reinterpret_cast<Memory<i64>*>(calloc(1, sizeof(Memory<i64>)));
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
