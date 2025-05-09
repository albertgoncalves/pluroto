#include <stdint.h>
#include <stdio.h>
#include <sys/mman.h>
#include <unistd.h>

// NOTE: See `https://www.cs.princeton.edu/~rs/talks/LLRB/LLRB.pdf`.
// NOTE: See `https://algs4.cs.princeton.edu/33balanced/RedBlackBST.java.html`.
// NOTE: See `http://www.mew.org/~kazu/proj/red-black-tree/`.

#define CAP_NODES (1 << 4)

typedef uint8_t  u8;
typedef uint32_t u32;
typedef size_t   usize;

typedef int32_t i32;
typedef ssize_t isize;

#define null nullptr

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

#define EXIT_IF(condition)                                                              \
    if (condition) {                                                                    \
        fprintf(stderr, "%s:%s:%d \"%s\"\n", __FILE__, __func__, __LINE__, #condition); \
        _exit(EXIT_FAILURE);                                                            \
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
    memory->root = null;
    memory->len_slots = 0;
    for (u32 i = 0; i < CAP_NODES; ++i) {
        memory->slots[memory->len_slots++] = &memory->nodes[i];
    }
}

template <typename K, typename V>
static Node<K, V>* alloc(Memory<K, V>* memory, K key, V value) {
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
static Node<K, V>* insert(Memory<K, V>* memory, Node<K, V>* node, K key, V value) {
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
static Node<K, V>* drop_min(Memory<K, V>* memory, Node<K, V>* node) {
    if (!node) {
        return null;
    }
    if (!node->left) {
        EXIT_IF(CAP_NODES <= memory->len_slots);
        memory->slots[memory->len_slots++] = node;
        return null;
    }
    if (!is_red(node->left) && !is_red(node->left->left)) {
        node = move_red_left(node);
    }
    node->left = drop_min(memory, node->left);
    return balance(node);
}

template <typename K, typename V>
static Node<K, V>* find_min(Node<K, V>* node) {
    if (!node->left) {
        return node;
    }
    return find_min(node->left);
}

template <typename K, typename V>
static Node<K, V>* drop(Memory<K, V>* memory, Node<K, V>* node, K key) {
    if (!node) {
        return null;
    }
    if (key < node->key) {
        if (node->left && !is_red(node->left) && !is_red(node->left->left)) {
            node = move_red_left(node);
        }
        node->left = drop(memory, node->left, key);
    } else {
        if (is_red(node->left)) {
            node = rotate_right(node);
        }
        if ((key == node->key) && (!node->right)) {
            EXIT_IF(CAP_NODES <= memory->len_slots);
            memory->slots[memory->len_slots++] = node;
            return null;
        }
        if (node->right && !is_red(node->right) && !is_red(node->right->left)) {
            node = move_red_right(node);
        }
        if (key == node->key) {
            Node<K, V>* min_node = find_min(node->right);
            node->key = min_node->key;
            node->value = min_node->value;
            node->right = drop_min(memory, node->right);
        } else {
            node->right = drop(memory, node->right, key);
        }
    }
    return balance(node);
}

template <typename K, typename V>
static void drop(Memory<K, V>* memory, K key) {
    memory->root = drop(memory, memory->root, key);
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

static void* alloc(usize size) {
    void* memory = mmap(null, size, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    EXIT_IF(memory == MAP_FAILED);
    return memory;
}

i32 main() {
    printf("sizeof(Node<u8, char>)   : %zu\n"
           "sizeof(Memory<u8, char>) : %zu\n",
           sizeof(Node<u8, char>),
           sizeof(Memory<u8, char>));
    Memory<u8, char>* memory = reinterpret_cast<Memory<u8, char>*>(alloc(sizeof(Memory<u8, char>)));
    {
        init(memory);
        for (u8 i = 0; i < 1; ++i) {
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
        drop<u8, char>(memory, 3);
        drop<u8, char>(memory, 14);
        insert<u8, char>(memory, 14, 'C');
        insert<u8, char>(memory, 3, 'D');
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
    return EXIT_SUCCESS;
}
