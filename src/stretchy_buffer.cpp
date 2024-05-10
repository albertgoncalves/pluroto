#include <cassert>
#include <cstdlib>
#include <iostream>

typedef size_t usize;

template <typename T>
struct Buffer {
    usize cap;
    usize len;
    T*    items;

    Buffer() : cap(0), len(0), items(nullptr) {
        std::cout << " - constructing object" << std::endl;
    }

    ~Buffer() {
        if (items != nullptr) {
            std::cout << " - freeing memory" << std::endl;
            free(items);
            cap = 0;
            len = 0;
            items = nullptr;
        }
    }

    void push(T x) {
        if (len == cap) {
            cap = ((cap | 1lu) + 7lu) & ~7lu;
            items = reinterpret_cast<T*>(reallocarray(items, cap, sizeof(T)));
            assert(items);
            std::cout << " - `cap` grown to " << cap << std::endl;
        }
        items[len++] = x;
    }

    T pop() {
        assert(len);
        return items[--len];
    }
};

int main() {
    Buffer<char> buffer{};
    buffer.push('H');
    buffer.push('i');
    buffer.push('!');
    buffer.push('\0');
    buffer.push('\0');
    buffer.pop();
    std::cout << buffer.items << std::endl;
    return 0;
}
