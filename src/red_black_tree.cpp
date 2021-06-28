#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// NOTE: See `https://www.cs.princeton.edu/~rs/talks/LLRB/LLRB.pdf`.
// NOTE: See `https://algs4.cs.princeton.edu/33balanced/RedBlackBST.java.html`.
// NOTE: See `http://www.mew.org/~kazu/proj/red-black-tree/`.

#define CAP_NODES (1 << 4)

typedef uint8_t  u8;
typedef uint32_t u32;
typedef size_t   usize;

typedef int32_t i32;

#define null nullptr

#define ERROR()                                                      \
    {                                                                \
        fprintf(stderr, "%s:%s:%d\n", __FILE__, __func__, __LINE__); \
        exit(EXIT_FAILURE);                                          \
    }

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

#define RED   true
#define BLACK false

template <typename K, typename V>
struct Node {
    Node* left;
    Node* right;
    K     key;
    V     value;
    bool  color;
};

template <typename K, typename V>
struct Memory {
    Node<K, V>* root;
    Node<K, V>  nodes[CAP_NODES];
    Node<K, V>* slots[CAP_NODES];
    u32         len_slots;
};

static void print(u8 x) {
    printf("%hhu", x);
}

static void print(char x) {
    printf("%c", x);
}

template <typename K, typename V>
static void init(Memory<K, V>* memory) {
    memory->len_slots = 0;
    for (u32 i = 0; i < CAP_NODES; ++i) {
        memory->slots[memory->len_slots++] = &memory->nodes[i];
    }
}

template <typename K, typename V>
bool find(Node<K, V>*, Node<K, V>*);

template <typename K, typename V>
bool find(Node<K, V>* node, Node<K, V>* pointer) {
    if (!node) {
        return false;
    }
    if (node == pointer) {
        return true;
    }
    return find(node->left, pointer) || find(node->right, pointer);
}

template <typename K, typename V>
static void collect(Memory<K, V>* memory) {
    memory->len_slots = 0;
    for (u32 i = 0; i < CAP_NODES; ++i) {
        Node<K, V>* pointer = &memory->nodes[i];
        // NOTE: This is *not* efficient.
        if (!find(memory->root, pointer)) {
            memory->slots[memory->len_slots++] = pointer;
        }
    }
}

template <typename K, typename V>
static Node<K, V>* alloc(Memory<K, V>* memory, K key, V value) {
    if (memory->len_slots == 0) {
        collect(memory);
    }
    EXIT_IF(memory->len_slots == 0);
    Node<K, V>* node = memory->slots[--memory->len_slots];
    node->key = key;
    node->value = value;
    node->color = RED;
    node->left = null;
    node->right = null;
    return node;
}

template <typename K, typename V>
static Node<K, V>* rotate_left(Node<K, V>* a) {
    EXIT_IF(!a);
    EXIT_IF(!a->right);
    Node<K, V>* b = a->right;
    a->right = b->left;
    b->left = a;
    b->color = a->color;
    a->color = RED;
    return b;
}

template <typename K, typename V>
static Node<K, V>* rotate_right(Node<K, V>* a) {
    EXIT_IF(!a);
    EXIT_IF(!a->left);
    Node<K, V>* b = a->left;
    a->left = b->right;
    b->right = a;
    b->color = a->color;
    a->color = RED;
    return b;
}

template <typename K, typename V>
static void flip(Node<K, V>* node) {
    EXIT_IF(!node);
    EXIT_IF(!node->left);
    EXIT_IF(!node->right);
    node->color = !node->color;
    node->left->color = !node->left->color;
    node->right->color = !node->right->color;
}

template <typename K, typename V>
static V* lookup(Node<K, V>* node, K key) {
    while (node) {
        if (key == node->key) {
            return &node->value;
        } else if (key < node->key) {
            node = node->left;
        } else {
            node = node->right;
        }
    }
    return null;
}

template <typename K, typename V>
static bool is_red(Node<K, V>* node) {
    if (!node) {
        return false;
    }
    return node->color == RED;
}

template <typename K, typename V>
static Node<K, V>* balance(Node<K, V>* node) {
    if (is_red(node->right) && !is_red(node->left)) {
        node = rotate_left(node);
    }
    if (is_red(node->left) && is_red(node->left->left)) {
        node = rotate_right(node);
    }
    if (is_red(node->left) && is_red(node->right)) {
        flip(node);
    }
    return node;
}

template <typename K, typename V>
Node<K, V>* insert(Memory<K, V>*, Node<K, V>*, K, V);

template <typename K, typename V>
Node<K, V>* insert(Memory<K, V>* memory, Node<K, V>* node, K key, V value) {
    if (!node) {
        return alloc(memory, key, value);
    }
    if (key == node->key) {
        node->value = value;
    } else if (key < node->key) {
        node->left = insert(memory, node->left, key, value);
    } else {
        node->right = insert(memory, node->right, key, value);
    }
    return balance(node);
}

template <typename K, typename V>
static void insert(Memory<K, V>* memory, K key, V value) {
    memory->root = insert(memory, memory->root, key, value);
    memory->root->color = BLACK;
}

template <typename K, typename V>
static Node<K, V>* move_red_left(Node<K, V>* node) {
    EXIT_IF(!node);
    EXIT_IF(!node->right);
    flip(node);
    if (is_red(node->right->left)) {
        node->right = rotate_right(node->right);
        node = rotate_left(node);
        flip(node);
    }
    return node;
}

template <typename K, typename V>
static Node<K, V>* move_red_right(Node<K, V>* node) {
    EXIT_IF(!node);
    EXIT_IF(!node->left);
    flip(node);
    if (is_red(node->left->left)) {
        node = rotate_right(node);
        flip(node);
    }
    return node;
}

template <typename K, typename V>
Node<K, V>* drop_min(Node<K, V>*);

template <typename K, typename V>
Node<K, V>* drop_min(Node<K, V>* node) {
    if (!node || !node->left) {
        return null;
    }
    if (!is_red(node->left) && !is_red(node->left->left)) {
        node = move_red_left(node);
    }
    node->left = drop_min(node->left);
    return balance(node);
}

template <typename K, typename V>
static void drop_min(Memory<K, V>* memory) {
    memory->root = drop_min(memory->root);
    if (memory->root) {
        memory->root->color = BLACK;
    }
}

template <typename K, typename V>
static Node<K, V>* find_min(Node<K, V>*);

template <typename K, typename V>
static Node<K, V>* find_min(Node<K, V>* node) {
    if (!node->left) {
        return node;
    }
    return find_min(node->left);
}

template <typename K, typename V>
Node<K, V>* drop(Node<K, V>*, K);

template <typename K, typename V>
Node<K, V>* drop(Node<K, V>* node, K key) {
    if (!node) {
        return null;
    }
    if (key < node->key) {
        if (node->left && !is_red(node->left) && !is_red(node->left->left)) {
            node = move_red_left(node);
        }
        node->left = drop(node->left, key);
    } else {
        if (is_red(node->left)) {
            node = rotate_right(node);
        }
        if ((key == node->key) && (!node->right)) {
            return null;
        }
        if (node->right && !is_red(node->right) && !is_red(node->right->left))
        {
            node = move_red_right(node);
        }
        if (key == node->key) {
            Node<K, V>* min_node = find_min(node->right);
            node->key = min_node->key;
            node->value = min_node->value;
            node->right = drop_min(node->right);
        } else {
            node->right = drop(node->right, key);
        }
    }
    return balance(node);
}

template <typename K, typename V>
static void drop(Memory<K, V>* memory, K key) {
    memory->root = drop(memory->root, key);
    if (memory->root) {
        memory->root->color = BLACK;
    }
}

template <typename K, typename V>
static void println(Node<K, V>* node, u8 n) {
    if (!node) {
        printf("%*s[ ]\n", n, "");
        return;
    }
    printf("%*s[%c] ", n, "", node->color == RED ? 'r' : 'b');
    print(node->key);
    printf(": ");
    print(node->value);
    printf("\n");
    n += 4;
    println(node->left, n);
    println(node->right, n);
}

i32 main() {
    printf("sizeof(Node<u8, char>)   : %zu\n"
           "sizeof(Memory<u8, char>) : %zu\n",
           sizeof(Node<u8, char>),
           sizeof(Memory<u8, char>));
    Memory<u8, char>* memory =
        reinterpret_cast<Memory<u8, char>*>(malloc(sizeof(Memory<u8, char>)));
    EXIT_IF(!memory);
    {
        init(memory);
        for (u8 i = 0; i < CAP_NODES; ++i) {
            insert<u8, char>(memory, i, 'a' + static_cast<char>(i));
        }
        for (u8 i = 0; i < 3; ++i) {
            drop<u8, char>(memory, i);
        }
        drop<u8, char>(memory, 0);
        drop<u8, char>(memory, 8);
        drop<u8, char>(memory, 12);
        drop<u8, char>(memory, 6);
        insert<u8, char>(memory, 15, '!');
        insert<u8, char>(memory, 12, '@');
        drop<u8, char>(memory, 14);
        drop<u8, char>(memory, 10);
        drop<u8, char>(memory, 11);
        insert<u8, char>(memory, 11, '#');
        insert<u8, char>(memory, 0, '$');
        insert<u8, char>(memory, 6, '%');
        insert<u8, char>(memory, 8, '^');
        insert<u8, char>(memory, 10, '&');
        insert<u8, char>(memory, 20, '*');
        drop<u8, char>(memory, 20);
        drop<u8, char>(memory, 0);
        insert<u8, char>(memory, 14, '*');
        insert<u8, char>(memory, 0, 'a');
        insert<u8, char>(memory, 1, '{');
        insert<u8, char>(memory, 2, '}');
        drop<u8, char>(memory, 7);
        drop<u8, char>(memory, 11);
        insert<u8, char>(memory, 11, 'A');
        insert<u8, char>(memory, 7, 'B');
        printf("\n");
        println(memory->root, 0);
        printf("\n");
        for (u8 i = 0; i < CAP_NODES; ++i) {
            printf("%4hhu: ", i);
            char* value = lookup(memory->root, i);
            if (value) {
                print(*value);
            } else {
                printf("_");
            }
            printf("\n");
        }
    }
    free(memory);
    return EXIT_SUCCESS;
}
