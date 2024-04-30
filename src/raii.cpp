#include <stdint.h>
#include <stdio.h>

typedef int32_t i32;

#define OK 0

template <typename T>
struct MyStruct {
    T x;
    MyStruct(T t) : x(t) {
        printf("Calling constructor\n");
    }
    ~MyStruct() {
        printf("Calling destructor\n");
    }
    MyStruct(MyStruct const&) = default;
};

i32 main() {
    auto my_struct = MyStruct<i32>(-123);
    printf("%d\n", my_struct.x);
    return OK;
}
