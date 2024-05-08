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

#define CAP_INSTS      (1 << 6)
#define CAP_NODES      (1 << 5)
#define CAP_HEAP       (1 << 5)
#define CAP_LABELS     (1 << 3)
#define CAP_UNRESOLVED (1 << 3)

template <typename T, u32 N>
struct Stack {
    T   items[N];
    u32 len;
};

enum InstTag {
    INST_HALT = 0,

    INST_PUSH,

    INST_COPY,
    INST_STORE,
    INST_DROP,
    INST_SWAP,

    INST_JUMP,
    INST_JIFZ,

    INST_NEW,
    INST_SV32,
    INST_RD32,

    INST_GT,

    INST_ADD,
    INST_SUB,
};

union Inst {
    InstTag as_tag;
    i32     as_i32;
};

union Node {
    i32 as_i32;
};

struct Unresolved {
    Inst* inst;
    u32*  label;
};

struct Vm {
    Stack<Inst, CAP_INSTS>            insts;
    Stack<Node, CAP_NODES>            nodes;
    Stack<u8, CAP_HEAP>               heap;
    Stack<u32, CAP_LABELS>            labels;
    Stack<Unresolved, CAP_UNRESOLVED> unresolved;
};

#define EXIT()                                                       \
    {                                                                \
        fflush(stdout);                                              \
        fprintf(stderr, "%s:%s:%d\n", __FILE__, __func__, __LINE__); \
        _exit(EXIT_FAILURE);                                         \
    }

#define EXIT_IF(condition)                                                              \
    if (condition) {                                                                    \
        fflush(stdout);                                                                 \
        fprintf(stderr, "%s:%s:%d \"%s\"\n", __FILE__, __func__, __LINE__, #condition); \
        _exit(EXIT_FAILURE);                                                            \
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

static void run(Vm* vm) {
    for (u32 i = 0; i < vm->unresolved.len; ++i) {
        Unresolved unresolved = vm->unresolved.items[i];
        unresolved.inst->as_i32 = static_cast<i32>(*unresolved.label);
    }
    for (u32 i = 0;;) {
        const Inst inst = get(&vm->insts, i++);
        switch (inst.as_tag) {
        case INST_HALT: {
            return;
        }
        case INST_PUSH: {
            alloc(&vm->nodes)->as_i32 = get(&vm->insts, i++).as_i32;
            break;
        }
        case INST_COPY: {
            alloc(&vm->nodes)->as_i32 =
                get(&vm->nodes, (vm->nodes.len - 1) - static_cast<u32>(get(&vm->insts, i++).as_i32))
                    .as_i32;
            break;
        }
        case INST_STORE: {
            const i32 x = pop(&vm->nodes).as_i32;
            const i32 offset = get(&vm->insts, i++).as_i32;
            get_pointer(&vm->nodes, (vm->nodes.len - 1) - static_cast<u32>(offset))->as_i32 = x;
            break;
        }
        case INST_DROP: {
            vm->nodes.len -= static_cast<u32>(get(&vm->insts, i++).as_i32);
            break;
        }
        case INST_SWAP: {
            const i32 r = pop(&vm->nodes).as_i32;
            const i32 l = pop(&vm->nodes).as_i32;
            alloc(&vm->nodes)->as_i32 = r;
            alloc(&vm->nodes)->as_i32 = l;
            break;
        }
        case INST_JUMP: {
            i = static_cast<u32>(pop(&vm->nodes).as_i32);
            break;
        }
        case INST_JIFZ: {
            const i32 x = pop(&vm->nodes).as_i32;
            const i32 insts_index = pop(&vm->nodes).as_i32;
            if (x == 0) {
                i = static_cast<u32>(insts_index);
            }
            break;
        }
        case INST_NEW: {
            alloc(&vm->nodes)->as_i32 = static_cast<i32>(
                alloc_offset(&vm->heap, static_cast<u32>(get(&vm->insts, i++).as_i32)));
            break;
        }
        case INST_SV32: {
            const i32 x = pop(&vm->nodes).as_i32;
            const i32 heap_index = pop(&vm->nodes).as_i32;
            const i32 offset = get(&vm->insts, i++).as_i32;
            *reinterpret_cast<i32*>(get_pointer(&vm->heap, static_cast<u32>(heap_index + offset))) =
                x;
            break;
        }
        case INST_RD32: {
            const i32 heap_index = pop(&vm->nodes).as_i32;
            const i32 offset = get(&vm->insts, i++).as_i32;
            alloc(&vm->nodes)->as_i32 = *reinterpret_cast<i32*>(
                get_pointer(&vm->heap, static_cast<u32>(heap_index + offset)));
            break;
        }
        case INST_GT: {
            const i32 r = pop(&vm->nodes).as_i32;
            const i32 l = pop(&vm->nodes).as_i32;
            alloc(&vm->nodes)->as_i32 = l > r ? 1 : 0;
            break;
        }
        case INST_ADD: {
            const i32 r = pop(&vm->nodes).as_i32;
            const i32 l = pop(&vm->nodes).as_i32;
            alloc(&vm->nodes)->as_i32 = l + r;
            break;
        }
        case INST_SUB: {
            const i32 r = pop(&vm->nodes).as_i32;
            const i32 l = pop(&vm->nodes).as_i32;
            alloc(&vm->nodes)->as_i32 = l - r;
            break;
        }
        default: {
            EXIT();
        }
        }
    }
}

template <InstTag X>
static void inst(Vm* vm) {
    Inst* inst = alloc(&vm->insts);
    inst->as_tag = X;
}

static void inst_i32(Vm* vm, i32 x) {
    Inst* inst = alloc(&vm->insts);
    inst->as_i32 = x;
}

static void inst_label(Vm* vm, u32 label_index) {
    EXIT_IF(CAP_LABELS <= label_index);
    Unresolved* unresolved = alloc(&vm->unresolved);
    unresolved->inst = alloc(&vm->insts);
    unresolved->label = &vm->labels.items[label_index];
}

static void set_label(Vm* vm, u32 label_index) {
    EXIT_IF(CAP_LABELS <= label_index);
    vm->labels.items[label_index] = vm->insts.len;
}

static void* alloc(usize size) {
    void* memory = mmap(null, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    EXIT_IF(memory == MAP_FAILED);
    return memory;
}

static void reset(Vm* vm) {
    vm->insts.len = 0;
    vm->nodes.len = 0;
    vm->heap.len = 0;
    vm->labels.len = 0;
    vm->unresolved.len = 0;
}

#define TEST(vm, insts_len, node_i32, heap_len)      \
    {                                                \
        EXIT_IF(vm->insts.len != insts_len);         \
        run(vm);                                     \
        EXIT_IF(pop(&vm->nodes).as_i32 != node_i32); \
        EXIT_IF(vm->nodes.len != 0);                 \
        EXIT_IF(vm->heap.len != heap_len);           \
        fputc('.', stdout);                          \
    }

static void test_0(Vm* vm) {
    reset(vm);
    {
        /*  i32 main() {
         *      return f();
         *  }
         */
        /*      push `0`
         *      push `1`
         *      jump
         *  `0`:
         *      halt
         */
        inst<INST_PUSH>(vm);
        inst_label(vm, 0);
        inst<INST_PUSH>(vm);
        inst_label(vm, 1);
        inst<INST_JUMP>(vm);
        set_label(vm, 0);
        inst<INST_HALT>(vm);

        /*  i32 f() {
         *      return -5 - 7;
         *  }
         */
        /*  `1`:
         *      push -5
         *      push 7
         *      sub
         *      swap
         *      jump
         */
        set_label(vm, 1);
        inst<INST_PUSH>(vm);
        inst_i32(vm, -5);
        inst<INST_PUSH>(vm);
        inst_i32(vm, 7);
        inst<INST_SUB>(vm);
        inst<INST_SWAP>(vm);
        inst<INST_JUMP>(vm);
    }
    TEST(vm, 13, -12, 0);
}

static void test_1(Vm* vm) {
    reset(vm);
    {
        /*      push `0`
         *      push `1`
         *      push 0
         *      jnz
         *  `0`:
         *      halt
         *  `1`:
         *      push -1
         */
        inst<INST_PUSH>(vm);
        inst_label(vm, 0);
        inst<INST_PUSH>(vm);
        inst_label(vm, 1);
        inst<INST_PUSH>(vm);
        inst_i32(vm, 1);
        inst<INST_JIFZ>(vm);
        set_label(vm, 0);
        inst<INST_HALT>(vm);
        set_label(vm, 1);
        inst<INST_PUSH>(vm);
        inst_i32(vm, -1);
    }
    TEST(vm, 10, 7, 0);
}

static void test_2(Vm* vm) {
    reset(vm);
    {
        /*  i32 main() {
         *      (void)new(4);
         *      i32 addr = new(4);
         *      return addr;
         *  }
         */
        /*      new 4
         *      drop 1
         *      new 4
         *      halt
         */
        inst<INST_NEW>(vm);
        inst_i32(vm, 4);
        inst<INST_DROP>(vm);
        inst_i32(vm, 1);
        inst<INST_NEW>(vm);
        inst_i32(vm, 4);
        inst<INST_HALT>(vm);
    }
    TEST(vm, 7, 4, 8);
}

static void test_3(Vm* vm) {
    reset(vm);
    {
        /*  i32 main() {
         *      (void)new(8);
         *      i32 addr = new(12);
         *      *(&HEAP[addr + 4] as i32*) = -123;
         *      return *(&HEAP[addr + 4] as i32*);
         *  }
         */
        /*      new 8
         *      drop 1
         *      new 12
         *      copy 0
         *      push -123
         *      sv32 4
         *      rd32 4
         *      halt
         */
        inst<INST_NEW>(vm);
        inst_i32(vm, 8);
        // [addr:0]

        inst<INST_DROP>(vm);
        inst_i32(vm, 1);
        // []

        inst<INST_NEW>(vm);
        inst_i32(vm, 12);
        // [addr:8]

        inst<INST_COPY>(vm);
        inst_i32(vm, 0);
        // [addr:8, addr:8]

        inst<INST_PUSH>(vm);
        inst_i32(vm, -123);
        // [addr:8, addr:8, -123]

        inst<INST_SV32>(vm);
        inst_i32(vm, 4);
        // [addr:8]

        inst<INST_RD32>(vm);
        inst_i32(vm, 4);
        // [?]

        inst<INST_HALT>(vm);
    }
    TEST(vm, 15, -123, 20);
}

static void test_4(Vm* vm) {
    reset(vm);
    {
        /*  i32 main() {
         *      i32 addr = new(12);
         *      *(&HEAP[addr + 0] as i32*) = f;
         *      *(&HEAP[addr + 4] as i32*) = -7;
         *      *(&HEAP[addr + 8] as i32*) = 6;
         *      return HEAP[addr + 0](addr);
         *  }
         */
        /*      new 12
         *
         *      copy 0
         *      push `0`
         *      sv32 0
         *
         *      copy 0
         *      push -7
         *      sv32 4
         *
         *      copy 0
         *      push 6
         *      sv32 8
         *
         *      push `1`
         *      copy 1
         *      copy 2
         *      rd32 0
         *      jump
         *  `1`:
         *      store 0
         *      halt
         */
        inst<INST_NEW>(vm);
        inst_i32(vm, 12);
        // [addr:0]

        inst<INST_COPY>(vm);
        inst_i32(vm, 0);
        // [addr:0, addr:0]

        inst<INST_PUSH>(vm);
        inst_label(vm, 0);
        // [addr:0, addr:0, f]

        inst<INST_SV32>(vm);
        inst_i32(vm, 0);
        // [addr:0]

        inst<INST_COPY>(vm);
        inst_i32(vm, 0);
        inst<INST_PUSH>(vm);
        inst_i32(vm, -7);
        inst<INST_SV32>(vm);
        inst_i32(vm, 4);

        inst<INST_COPY>(vm);
        inst_i32(vm, 0);
        inst<INST_PUSH>(vm);
        inst_i32(vm, 6);
        inst<INST_SV32>(vm);
        inst_i32(vm, 8);

        inst<INST_PUSH>(vm);
        inst_label(vm, 1);
        // [addr:0, return]

        inst<INST_COPY>(vm);
        inst_i32(vm, 1);
        // [addr:0, return, addr:0]

        inst<INST_COPY>(vm);
        inst_i32(vm, 2);
        // [addr:0, return, addr:0, addr:0]

        inst<INST_RD32>(vm);
        inst_i32(vm, 0); // instruction address of `f`
        // [addr:0, return, addr:0, f]

        inst<INST_JUMP>(vm); // f(addr)
        // [addr:0, return, addr:0]

        set_label(vm, 1);
        inst<INST_STORE>(vm);
        inst_i32(vm, 0);

        inst<INST_HALT>(vm);

        /*  i32 f(i32 addr) {
         *      return HEAP[addr + 4] - HEAP[addr + 8];
         *  }
         */
        /*  `0`:
         *      copy 0
         *      rd32 4
         *      copy 1
         *      rd32 8
         *      sub
         *      store 0
         *      swap
         *      jump
         */
        set_label(vm, 0);
        inst<INST_COPY>(vm);
        inst_i32(vm, 0);
        // [..., return, addr, addr]

        inst<INST_RD32>(vm);
        inst_i32(vm, 4);
        // [..., return, addr, HEAP[addr + 4]]

        inst<INST_COPY>(vm);
        inst_i32(vm, 1);
        // [..., return, addr, HEAP[addr + 4], addr]

        inst<INST_RD32>(vm);
        inst_i32(vm, 8);
        // [..., return, addr, HEAP[addr + 4], HEAP[addr + 8]]

        inst<INST_SUB>(vm);
        // [..., return, addr, HEAP[addr + 4]-HEAP[addr + 8]]

        inst<INST_STORE>(vm);
        inst_i32(vm, 0);
        // [..., return, HEAP[addr + 4]-HEAP[addr + 8]]

        inst<INST_SWAP>(vm);
        // [..., HEAP[addr + 4]-HEAP[addr + 8], return]

        inst<INST_JUMP>(vm);
        // [..., HEAP[addr + 4]-HEAP[addr + 8]]
    }
    TEST(vm, 45, -13, 12);
}

i32 main() {
    printf("\n"
           "sizeof(Inst)       : %zu\n"
           "sizeof(Node)       : %zu\n"
           "sizeof(Unresolved) : %zu\n"
           "sizeof(Vm)         : %zu\n"
           "\n",
           sizeof(Inst),
           sizeof(Node),
           sizeof(Unresolved),
           sizeof(Vm));
    Vm* vm = reinterpret_cast<Vm*>(alloc(sizeof(Vm)));
    test_0(vm);
    test_1(vm);
    test_2(vm);
    test_3(vm);
    test_4(vm);
    printf("\nDone!\n");
    return EXIT_SUCCESS;
}
