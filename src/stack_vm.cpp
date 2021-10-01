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

template <typename T, u32 N>
struct Stack {
    T   items[N];
    u32 len;
};

enum InstTag {
    INST_HALT = 0,
    INST_PUSH,
    INST_SUB,
    INST_JNZ,
    INST_SWAP,
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
};

#define EXIT()                                                       \
    {                                                                \
        fprintf(stderr, "%s:%s:%d\n", __FILE__, __func__, __LINE__); \
        _exit(EXIT_FAILURE);                                         \
    }

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

template <typename T, u32 N>
static T* alloc(Stack<T, N>* stack) {
    EXIT_IF(N <= stack->len);
    return &stack->items[stack->len++];
}

template <typename T, u32 N>
static T get(const Stack<T, N>* stack, u32 i) {
    EXIT_IF(stack->len <= i);
    return stack->items[i];
}

template <typename T, u32 N>
static T pop(Stack<T, N>* stack) {
    EXIT_IF(stack->len == 0);
    return stack->items[--stack->len];
}

static void run(Memory* memory) {
    for (u32 i = 0;;) {
        const Inst inst = get(&memory->insts, i);
        switch (inst.as_tag) {
        case INST_HALT: {
            return;
        }
        case INST_PUSH: {
            alloc(&memory->nodes)->as_i32 = get(&memory->insts, ++i).as_i32;
            ++i;
            break;
        }
        case INST_SUB: {
            const i32 r = pop(&memory->nodes).as_i32;
            const i32 l = pop(&memory->nodes).as_i32;
            alloc(&memory->nodes)->as_i32 = l - r;
            ++i;
            break;
        }
        case INST_JNZ: {
            const i32 jump = pop(&memory->nodes).as_i32;
            const i32 index = pop(&memory->nodes).as_i32;
            i = jump ? static_cast<u32>(index) : i + 1;
            break;
        }
        case INST_SWAP: {
            const i32 r = pop(&memory->nodes).as_i32;
            const i32 l = pop(&memory->nodes).as_i32;
            alloc(&memory->nodes)->as_i32 = r;
            alloc(&memory->nodes)->as_i32 = l;
            ++i;
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
}

#define RUN(memory)                                 \
    {                                               \
        run(memory);                                \
        printf("%d\n", pop(&memory->nodes).as_i32); \
        EXIT_IF(memory->nodes.len != 0);            \
    }

static void test_0(Memory* memory) {
    reset(memory);
    {
        inst<INST_PUSH>(memory);
        inst_i32(memory, 7);
        inst<INST_PUSH>(memory);
        inst_i32(memory, 8);
        inst<INST_PUSH>(memory);
        inst_i32(memory, 1);
        inst<INST_JNZ>(memory);
        inst<INST_HALT>(memory);

        inst<INST_PUSH>(memory);
        inst_i32(memory, -5);
        inst<INST_PUSH>(memory);
        inst_i32(memory, 7);
        inst<INST_SUB>(memory);
        inst<INST_SWAP>(memory);
        inst<INST_PUSH>(memory);
        inst_i32(memory, 1);
        inst<INST_JNZ>(memory);

        EXIT_IF(memory->insts.len != 17);
    }
    RUN(memory);
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

        EXIT_IF(memory->insts.len != 8);
    }
    RUN(memory);
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
    printf("Done!\n");
    return EXIT_SUCCESS;
}
