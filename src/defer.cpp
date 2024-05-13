#include <cstdint>
#include <cstdio>
#include <functional>

typedef int32_t i32;

struct Defer {
    std::function<void(void)> f;
    ~Defer() {
        f();
    }
};

#define IDENT_1(k) __##k##__
#define IDENT_2(k) IDENT_1(k)
#define IDENT_3()  IDENT_2(__COUNTER__)

#define defer(block)  \
    Defer IDENT_3() { \
        [&]() block   \
    }

i32 main() {
    {
        defer({ printf("(0) defer(...)\n"); });
    }
    defer({
        printf("(4) defer(...)\n");
        printf("(5) defer(...)\n");
    });
    printf("(1) ...\n");
    defer({ printf("(3) defer(...)\n"); });
    printf("(2) return ...;\n");
    return 0;
}
