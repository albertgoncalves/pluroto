#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>

typedef uint8_t  u8;
typedef uint32_t u32;
typedef size_t   usize;

typedef int32_t i32;
typedef ssize_t isize;

#define null nullptr

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

#define CAP_INSTS (1 << 5)
#define CAP_NODES (1 << 5)
#define CAP_HEAP  (1 << 5)

template <typename T, u32 N>
struct Stack {
    T   items[N];
    u32 len;
};

enum InstTag {
    INST_HALT = 0,
    INST_DUP,
    INST_DROP,
    INST_PUSH,
    INST_SUB,
    INST_JNZ,
    INST_SWAP,
    INST_NEW,
    INST_SV32,
    INST_RD32,
};

union Inst {
    InstTag as_tag;
    i32     as_i32;
};

union Node {
    i32 as_i32;
};

struct Memory {
    Stack<Inst, CAP_INSTS> insts;
    Stack<Node, CAP_NODES> nodes;
    Stack<u8, CAP_HEAP>    heap;
};

#define EXIT()                                                       \
    {                                                                \
        fflush(stdout);                                              \
        fprintf(stderr, "%s:%s:%d\n", __FILE__, __func__, __LINE__); \
        _exit(EXIT_FAILURE);                                         \
    }

#define EXIT_IF(condition)           \
    if (condition) {                 \
        fflush(stdout);              \
        fprintf(stderr,              \
                "%s:%s:%d \"%s\"\n", \
                __FILE__,            \
                __func__,            \
                __LINE__,            \
                #condition);         \
        _exit(EXIT_FAILURE);         \
    }

template <typename T, u32 N>
static T* alloc(Stack<T, N>* stack) {
    EXIT_IF(N <= stack->len);
    return &stack->items[stack->len++];
}

template <typename T, u32 N>
static T* alloc(Stack<T, N>* stack, u32 n) {
    EXIT_IF(n == 0);
    u32 len = stack->len + n;
    EXIT_IF(N < len);
    T* x = &stack->items[stack->len];
    stack->len = len;
    return x;
}

template <typename T, u32 N>
static u32 alloc_offset(Stack<T, N>* stack, u32 n) {
    return static_cast<u32>(alloc(stack, n) - &stack->items[0]);
}

template <typename T, u32 N>
static T get(const Stack<T, N>* stack, u32 i) {
    EXIT_IF(stack->len <= i);
    return stack->items[i];
}

template <typename T, u32 N>
static T* get_pointer(Stack<T, N>* stack, u32 i) {
    EXIT_IF(stack->len <= i);
    return &stack->items[i];
}

template <typename T, u32 N>
static T pop(Stack<T, N>* stack) {
    EXIT_IF(stack->len == 0);
    return stack->items[--stack->len];
}

static void run(Memory* memory) {
    for (u32 i = 0;;) {
        const Inst inst = get(&memory->insts, i++);
        switch (inst.as_tag) {
        case INST_HALT: {
            return;
        }
        case INST_DUP: {
            i32 x = pop(&memory->nodes).as_i32;
            alloc(&memory->nodes)->as_i32 = x;
            alloc(&memory->nodes)->as_i32 = x;
            break;
        }
        case INST_DROP: {
            (void)pop(&memory->nodes).as_i32;
            break;
        }
        case INST_PUSH: {
            alloc(&memory->nodes)->as_i32 = get(&memory->insts, i++).as_i32;
            break;
        }
        case INST_SUB: {
            const i32 r = pop(&memory->nodes).as_i32;
            const i32 l = pop(&memory->nodes).as_i32;
            alloc(&memory->nodes)->as_i32 = l - r;
            break;
        }
        case INST_JNZ: {
            const i32 jump = pop(&memory->nodes).as_i32;
            const i32 insts_index = pop(&memory->nodes).as_i32;
            if (jump) {
                i = static_cast<u32>(insts_index);
            }
            break;
        }
        case INST_SWAP: {
            const i32 r = pop(&memory->nodes).as_i32;
            const i32 l = pop(&memory->nodes).as_i32;
            alloc(&memory->nodes)->as_i32 = r;
            alloc(&memory->nodes)->as_i32 = l;
            break;
        }
        case INST_NEW: {
            alloc(&memory->nodes)->as_i32 = static_cast<i32>(alloc_offset(
                &memory->heap,
                static_cast<u32>(get(&memory->insts, i++).as_i32)));
            break;
        }
        case INST_SV32: {
            const i32 x = pop(&memory->nodes).as_i32;
            const i32 heap_index = pop(&memory->nodes).as_i32;
            const i32 offset = get(&memory->insts, i++).as_i32;
            // NOTE: Bounds check?
            i32* bytes = reinterpret_cast<i32*>(
                &memory->heap.items[heap_index + offset]);
            *bytes = x;
            break;
        }
        case INST_RD32: {
            const i32 heap_index = pop(&memory->nodes).as_i32;
            const i32 offset = get(&memory->insts, i++).as_i32;
            alloc(&memory->nodes)->as_i32 = *reinterpret_cast<i32*>(
                get_pointer(&memory->heap,
                            static_cast<u32>(heap_index + offset)));
            break;
        }
        default: {
            EXIT();
        }
        }
    }
}

template <InstTag X>
static void inst(Memory* memory) {
    Inst* inst = alloc(&memory->insts);
    inst->as_tag = X;
}

static void inst_i32(Memory* memory, i32 x) {
    Inst* inst = alloc(&memory->insts);
    inst->as_i32 = x;
}

static void* alloc(usize size) {
    void* memory = mmap(null,
                        size,
                        PROT_READ | PROT_WRITE,
                        MAP_ANONYMOUS | MAP_PRIVATE,
                        -1,
                        0);
    EXIT_IF(memory == MAP_FAILED);
    return memory;
}

static void reset(Memory* memory) {
    memory->insts.len = 0;
    memory->nodes.len = 0;
    memory->heap.len = 0;
}

#define TEST(memory, insts_len, node_i32, heap_len)      \
    {                                                    \
        EXIT_IF(memory->insts.len != insts_len);         \
        run(memory);                                     \
        EXIT_IF(pop(&memory->nodes).as_i32 != node_i32); \
        EXIT_IF(memory->nodes.len != 0);                 \
        EXIT_IF(memory->heap.len != heap_len);           \
        fputc('.', stdout);                              \
    }

static void test_0(Memory* memory) {
    reset(memory);
    {
        /*  i32 main() {
         *      return f();
         *  }
         */
        inst<INST_PUSH>(memory);
        inst_i32(memory, 7);
        inst<INST_PUSH>(memory);
        inst_i32(memory, 8);
        inst<INST_PUSH>(memory);
        inst_i32(memory, 1);
        inst<INST_JNZ>(memory);
        inst<INST_HALT>(memory);

        /*  i32 f() {
         *      return -5 - 7;
         *  }
         */
        inst<INST_PUSH>(memory);
        inst_i32(memory, -5);
        inst<INST_PUSH>(memory);
        inst_i32(memory, 7);
        inst<INST_SUB>(memory);
        inst<INST_SWAP>(memory);
        inst<INST_PUSH>(memory);
        inst_i32(memory, 1);
        inst<INST_JNZ>(memory);
    }
    TEST(memory, 17, -12, 0);
}

static void test_1(Memory* memory) {
    reset(memory);
    {
        inst<INST_PUSH>(memory);
        inst_i32(memory, 7);
        inst<INST_PUSH>(memory);
        inst_i32(memory, 8);
        inst<INST_PUSH>(memory);
        inst_i32(memory, 0);
        inst<INST_JNZ>(memory);
        inst<INST_HALT>(memory);
    }
    TEST(memory, 8, 7, 0);
}

static void test_2(Memory* memory) {
    reset(memory);
    {
        /*  i32 main() {
         *      (void)new(4);
         *      i32 addr = new(4);
         *      return addr;
         *  }
         */
        inst<INST_NEW>(memory);
        inst_i32(memory, 4);
        inst<INST_DROP>(memory);
        inst<INST_NEW>(memory);
        inst_i32(memory, 4);
        inst<INST_HALT>(memory);
    }
    TEST(memory, 6, 4, 8);
}

static void test_3(Memory* memory) {
    reset(memory);
    {
        /*  i32 main() {
         *      (void)new(8);
         *      i32 addr = new(12);
         *      *(&HEAP[addr + 4] as i32*) = -123;
         *      return *(&HEAP[addr + 4] as i32*);
         *  }
         */
        inst<INST_NEW>(memory);
        inst_i32(memory, 8);
        // [heap_index:0]

        inst<INST_DROP>(memory);
        // []

        inst<INST_NEW>(memory);
        inst_i32(memory, 12);
        // [heap_index:8]

        inst<INST_PUSH>(memory);
        inst_i32(memory, -123);
        // [heap_index:0, x:-123]

        inst<INST_SV32>(memory);
        inst_i32(memory, 4);
        // []

        inst<INST_PUSH>(memory);
        inst_i32(memory, 8);
        // [heap_index:8]

        inst<INST_RD32>(memory);
        inst_i32(memory, 4);
        // [x:?]

        inst<INST_HALT>(memory);
    }
    TEST(memory, 14, -123, 20);
}

i32 main() {
    printf("\n"
           "sizeof(Inst)     : %zu\n"
           "sizeof(Node)     : %zu\n"
           "sizeof(Memory)   : %zu\n"
           "\n",
           sizeof(Inst),
           sizeof(Node),
           sizeof(Memory));
    Memory* memory = reinterpret_cast<Memory*>(alloc(sizeof(Memory)));
    test_0(memory);
    test_1(memory);
    test_2(memory);
    test_3(memory);
    printf("\nDone!\n");
    return EXIT_SUCCESS;
}
