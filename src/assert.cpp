#include <cassert>
#include <iostream>

static void f(int x) {
    int xs[2] = {0, 1};
    xs[1] = x;
    assert(xs[1] == 1);
}

int main() {
    const int x = 2;

    static const char* message = "Hello, world?";
    std::cout << message << '\n';

    f(x);

    return 0;
}
