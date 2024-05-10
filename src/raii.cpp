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
};

int main() {
    MyStruct<int> my_struct(-123);
    std::cout << my_struct.x << std::endl;

    try {
        MyStruct<int> _(456);
        if (0 != 1) {
            throw __LINE__;
        }
    } catch (int e) {
        std::cout << __FILE__ << ":" << e << std::endl;
        return 1;
    }

    return 0;
}
