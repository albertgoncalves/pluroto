#include <exception>
#include <iostream>

template <typename T>
struct MyStruct {
    T x;
    MyStruct(T t) : x(t) {
        std::cout << "Calling constructor" << std::endl;
    }
    ~MyStruct() {
        std::cout << "Calling destructor" << std::endl;
    }
    MyStruct(MyStruct const&) = default;
};

int main() {
    auto my_struct = MyStruct<int>(-123);
    std::cout << my_struct.x << std::endl;

    try {
        auto _ = MyStruct<int>(456);
        if (0 != 1) {
            throw __LINE__;
        }
    } catch (int e) {
        std::cout << __FILE__ << ":" << e << std::endl;
        return 1;
    }

    return 0;
}
