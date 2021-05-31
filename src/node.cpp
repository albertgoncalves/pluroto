#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef uint32_t u32;
typedef int32_t  i32;

template <typename T> struct Node {
    Node<T>* next;
    T        item;
};

#define SHOW(type, fmt)        \
    static void show(type x) { \
        printf(fmt, x);        \
    }

SHOW(u32, "u32: %u")
SHOW(i32, "i32: %d")

template <typename T> static void print_nodes(const Node<T>* node) {
    while (node) {
        printf("{ ");
        show(node->item);
        printf(", self*: %14p, next*: %14p }\n",
               reinterpret_cast<void*>(const_cast<Node<T>*>(node)),
               reinterpret_cast<void*>(const_cast<Node<T>*>(node->next)));
        node = node->next;
    }
}

#define N 5

#define DEMO(type)                                       \
    {                                                    \
        printf("\n");                                    \
        Node<type> nodes[N] = {};                        \
        for (type i = 0, j = 1; i < (N - 1); ++i, ++j) { \
            nodes[i].next = &nodes[j];                   \
            nodes[j].item = j;                           \
        }                                                \
        print_nodes(nodes);                              \
    }

i32 main() {
    printf("sizeof(Node<u32>) : %zu\n"
           "sizeof(Node<i32>) : %zu\n",
           sizeof(Node<u32>),
           sizeof(Node<i32>));
    DEMO(i32);
    DEMO(u32);
    return EXIT_SUCCESS;
}
