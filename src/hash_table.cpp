#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef uint8_t  u8;
typedef uint32_t u32;

typedef int32_t i32;

#define null nullptr

#define CAP_ITEMS 19

template <typename K, typename V>
struct Item {
    K    key;
    V    value;
    bool alive;
};

struct Count {
    u32 collisions;
    u32 purge_relocations;
    u32 purge_steps;
};

template <typename K, typename V>
struct Table {
    Item<K, V> items[CAP_ITEMS];
    u32        len;
    Count      count;
};

struct String {
    const char* chars;
    u32         len;
};

#define TO_STR(literal)      \
    ((String){               \
        literal,             \
        sizeof(literal) - 1, \
    })

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

#define FNV_32_PRIME        16777619
#define FNV_32_OFFSET_BASIS 2166136261

static u32 fnv_1a_32(const u8* bytes, u32 len) {
    u32 hash = FNV_32_OFFSET_BASIS;
    for (u32 i = 0; i < len; ++i) {
        hash ^= bytes[i];
        hash *= FNV_32_PRIME;
    }
    return hash;
}

static u32 hash(String string) {
    return fnv_1a_32(reinterpret_cast<const u8*>(string.chars), string.len);
}

static bool eq(String a, String b) {
    return ((a.len == b.len) && (!memcmp(a.chars, b.chars, a.len)));
}

template <typename K, typename V>
static u32 find_slot(Table<K, V>* table, K key) {
    u32 h = hash(key);
    for (u32 i = 0; i < CAP_ITEMS; ++i) {
        u32 j = (h + i) % CAP_ITEMS;
        if ((!table->items[j].alive) || eq(table->items[j].key, key)) {
            table->count.collisions += i;
            return j;
        }
    }
    ERROR();
}

template <typename K, typename V>
static V* lookup(Table<K, V>* table, K key) {
    Item<K, V>* item = &table->items[find_slot(table, key)];
    if (item->alive) {
        return &item->value;
    }
    return null;
}

template <typename K, typename V>
static void insert(Table<K, V>* table, K key, V value) {
    EXIT_IF(CAP_ITEMS <= table->len);
    u32 i = find_slot(table, key);
    if (!table->items[i].alive) {
        ++table->len;
    }
    table->items[i] = {key, value, true};
    return;
}

// NOTE: See `https://en.wikipedia.org/wiki/Open_addressing`.
template <typename K, typename V>
static void purge(Table<K, V>* table, K key) {
    u32 i = find_slot(table, key);
    if (!table->items[i].alive) {
        return;
    }
    u32 j = i;
    --table->len;
    for (;;) {
        table->items[i].alive = false;
        for (;;) {
            ++table->count.purge_steps;
            j = (j + 1) % CAP_ITEMS;
            if (!table->items[j].alive) {
                return;
            }
            u32 h = hash(table->items[j].key) % CAP_ITEMS;
            if (!(i <= j ? (i < h) && (h <= j) : (i < h) || (h <= j))) {
                break;
            }
        }
        ++table->count.purge_relocations;
        table->items[i] = table->items[j];
        i = j;
    }
}

static void print(String string) {
    printf("\"%.*s\"", string.len, string.chars);
}

static void print(i32 x) {
    printf("%d", x);
}

template <typename K, typename V>
static void println(Table<K, V>* table) {
    printf("{\n");
    for (u32 i = 0; i < CAP_ITEMS; ++i) {
        Item<K, V> item = table->items[i];
        printf("%5u  ", i);
        if (item.alive) {
            print(item.key);
            printf(" : ");
            print(item.value);
            printf("\n");
        } else {
            printf("_\n");
        }
    }
    printf("}\n");
}

i32 main() {
    printf("\n"
           "sizeof(String)             : %zu\n"
           "sizeof(Item<String, i32>)  : %zu\n"
           "sizeof(Table<String, i32>) : %zu\n"
           "\n",
           sizeof(String),
           sizeof(Item<String, i32>),
           sizeof(Table<String, i32>));
    Table<String, i32>* table = reinterpret_cast<Table<String, i32>*>(
        calloc(1, sizeof(Table<String, i32>)));
    insert(table, TO_STR("abd"), 2);
    insert(table, TO_STR("__?__"), 99);
    insert(table, TO_STR("001442"), 7);
    insert(table, TO_STR("bcdefg"), -1);
    insert(table, TO_STR("__?__"), 8);
    purge(table, TO_STR("001442"));
    purge(table, TO_STR("__?__"));
    purge(table, TO_STR("abd"));
    insert(table, TO_STR("__?__"), -2);
    insert(table, TO_STR("001442"), 98);
    insert(table, TO_STR("001443"), 97);
    insert(table, TO_STR("b"), -11);
    insert(table, TO_STR("c"), -12);
    insert(table, TO_STR("a"), -10);
    insert(table, TO_STR("d"), -13);
    insert(table, TO_STR("e"), -14);
    purge(table, TO_STR("__?__"));
    purge(table, TO_STR("a"));
    purge(table, TO_STR("b"));
    purge(table, TO_STR("c"));
    insert(table, TO_STR("a"), -10);
    insert(table, TO_STR("d"), 13);
    insert(table, TO_STR("a"), 100);
    insert(table, TO_STR("001442"), 980);
    insert(table, TO_STR("bcdefg"), 222);
    insert(table, TO_STR("e"), -141);
    insert(table, TO_STR("__?__"), -12345);
    println(table);
    {
        String key = TO_STR("__?__");
        i32*   value = lookup(table, key);
        printf("\n");
        print(key);
        printf(" : ");
        if (value) {
            printf("%d\n", *value);
        } else {
            printf("_\n");
        }
        printf("\n"
               "table->len                     : %u\n"
               "table->count.collisions        : %u\n"
               "table->count.purge_relocations : %u\n"
               "table->count.purge_steps       : %u\n",
               table->len,
               table->count.collisions,
               table->count.purge_relocations,
               table->count.purge_steps);
    }
    free(table);
    return EXIT_SUCCESS;
}
