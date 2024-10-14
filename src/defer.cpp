#include <functional>
#include <iostream>

struct Defer {
    std::function<void(void)> f;
    ~Defer() {
        f();
    }
};

#define IDENT_0(k) __##k##__
#define IDENT_1(k) IDENT_0(k)
#define IDENT_2()  IDENT_1(__COUNTER__)

#define defer(block)  \
    Defer IDENT_2() { \
        [&]() block   \
    }

int main() {
    {
        defer({ std::cout << "(0) defer(...)" << std::endl; });
    }

    defer({
        std::cout << "(4) defer(...)" << std::endl;
        std::cout << "(5) defer(...)" << std::endl;
    });

    std::cout << "(1) ..." << std::endl;

    defer({ std::cout << "(3) defer(...)" << std::endl; });

    std::cout << "(2) return ...;" << std::endl;

    return 0;
}
