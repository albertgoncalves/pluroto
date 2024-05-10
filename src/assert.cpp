#include <cassert>
#include <cstring>
#include <iostream>

static void f(int x) {
    int xs[2] = {0, 1};
    xs[1] = x;
    assert(xs[1] == 1);
}

int main() {
    const int x = 2;

    char message[14] = {};
    std::memcpy(message, "Hello, world?", 14);
    message[12] = '!';
    std::cout << message << std::endl;

    f(x);

    return 0;
}
