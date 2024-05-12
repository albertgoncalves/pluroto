#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <iostream>

typedef int32_t  i32;
typedef uint32_t u32;

template <typename T>
struct Buffer {
    u32 cap;
    u32 len;
    T*  items;

    Buffer() : cap(0), len(0), items(nullptr) {
        std::cout << " - constructing object" << std::endl;
    }

    Buffer(Buffer&& buffer) = default;

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

static std::ostream& operator<<(std::ostream& stream, Buffer<char>& buffer) {
    for (u32 i = 0; i < buffer.len; ++i) {
        stream << buffer.items[i];
    }
    return stream;
}

static Buffer<char> f() {
    Buffer<char> buffer{};
    return buffer;
}

i32 main() {
    std::cout << "sizeof(Buffer<char>): " << sizeof(Buffer<char>) << std::endl;

    auto buffer = f();
    buffer.push('H');
    buffer.push('i');
    buffer.push('!');
    buffer.push('\0');

    std::cout << buffer.items << std::endl;

    buffer.pop();
    std::cout << buffer << std::endl;

    return 0;
}
