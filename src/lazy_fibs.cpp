#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

#define CAP_LISTS (1 << 6)
#define CAP_LAZYS (1 << 6)
#define CAP_ARGS  (1 << 6)

typedef size_t usize;

typedef int32_t i32;
typedef int64_t i64;
typedef ssize_t isize;

#define null nullptr

#define EXIT_IF(condition)           \
    if (condition) {                 \
        fprintf(stderr,              \
                "%s:%s:%d \"%s\"\n", \
                __FILE__,            \
                __func__,            \
                __LINE__,            \
                #condition);         \
        _exit(EXIT_FAILURE);         \
    }

template <typename T>
struct LazyDefer {
    T (*func)(void*);
    void* payload;
};

template <typename T>
union LazyBody {
    LazyDefer<T> as_defer;
    T            as_result;
};

template <typename T>
struct Lazy {
    LazyBody<T> body;
    bool        cached;
};

template <typename T>
struct List {
    Lazy<List<T>*>* thunk;
    T               value;
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
    Buffer<List<T>, CAP_LISTS>        lists;
    Buffer<Lazy<List<T>*>, CAP_LAZYS> lazys;
    Buffer<Args<T>, CAP_ARGS>         args;
};

static List<i64>* FIBS = null;

template <typename T, usize N>
static T* alloc(Buffer<T, N>* buffer) {
    EXIT_IF(N <= buffer->len);
    return &buffer->items[buffer->len++];
}

template <typename T>
static T force(Lazy<T>* lazy) {
    if (lazy->cached) {
        return lazy->body.as_result;
    }
    lazy->cached = true;
    lazy->body.as_result =
        lazy->body.as_defer.func(lazy->body.as_defer.payload);
    return lazy->body.as_result;
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
static List<T>* drop(i32 n, List<T>* list) {
    for (; 0 < n; --n) {
        list = tail(list);
    }
    return list;
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
    list->thunk = alloc(&memory->lazys);
    list->thunk->cached = false;
    list->thunk->body.as_defer.func = f0;
    list->thunk->body.as_defer.payload = reinterpret_cast<void*>(args);
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
    list->thunk = alloc(&memory->lazys);
    list->thunk->cached = false;
    list->thunk->body.as_defer.func = f1;
    list->thunk->body.as_defer.payload = payload;
    return list;
}

static void* alloc(usize size) {
    void* memory = sbrk(static_cast<isize>(size));
    EXIT_IF(memory == reinterpret_cast<void*>(-1));
    memset(memory, 0, size);
    return memory;
}

i32 main() {
    printf("sizeof(List<i64>)             : %zu\n"
           "sizeof(LazyDefer<List<i64>*>) : %zu\n"
           "sizeof(LazyBody<List<i64>*>)  : %zu\n"
           "sizeof(Lazy<List<i64>*>)      : %zu\n"
           "sizeof(Args<i64>)             : %zu\n"
           "sizeof(Memory<i64>)           : %zu\n"
           "\n",
           sizeof(List<i64>),
           sizeof(LazyDefer<List<i64>*>),
           sizeof(LazyBody<List<i64>*>),
           sizeof(Lazy<List<i64>*>),
           sizeof(Args<i64>),
           sizeof(Memory<i64>));
    Memory<i64>* memory =
        reinterpret_cast<Memory<i64>*>(alloc(sizeof(Memory<i64>)));
    {
        FIBS = alloc(&memory->lists);
        FIBS->value = 0;
        FIBS->thunk = alloc(&memory->lazys);
        FIBS->thunk->cached = false;
        FIBS->thunk->body.as_defer.func = f2;
        FIBS->thunk->body.as_defer.payload = reinterpret_cast<void*>(memory);
        i64 x = head(drop(50, FIBS));
        printf("%ld\n", x);
        EXIT_IF(x != 12586269025);
    }
    return EXIT_SUCCESS;
}
