#include <stdint.h>
#include <stdio.h>

typedef int32_t i32;

#define OK 0

template <typename F>
struct __Defer__ {
    F f;
    __Defer__(__Defer__ const&) = default;
    __Defer__(F x) : f(x) {
    }
    ~__Defer__() {
        f();
    }
};

template <typename F>
__Defer__<F> __defer__(F f) {
    return __Defer__<F>(f);
}

#define __DEFER_1__(k) __##k##__
#define __DEFER_2__(k) __DEFER_1__(k)
#define __DEFER_3__()  __DEFER_2__(__COUNTER__)

#define DEFER(block) auto __DEFER_3__() = __defer__([&]() { block; })

i32 main() {
    {
        DEFER({ printf("(0) defer(...)\n"); });
    }
    DEFER({
        printf("(4) defer(...)\n");
        printf("(5) defer(...)\n");
    });
    printf("(1) ...\n");
    DEFER({ printf("(3) defer(...)\n"); });
    printf("(2) return ...;\n");
    return OK;
}
