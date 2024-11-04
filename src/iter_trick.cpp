#include <array>
#include <iostream>

template <typename T, size_t N>
static void iter(std::array<T, N> array, size_t i) {
    if (i == 0) {
        return;
    }

    bool condition = (array[--i] % 2) != 0;

    // NOTE: Calling `iter` first will print the array in its original order, despite starting from
    // the end of the array.
    iter(array, i);
    if (condition) {
        std::cout << ' ' << array[i];
    }
}

#define N 10

int main() {
    std::array<int, N> array;
    for (size_t i = 0; i < N; ++i) {
        array[i] = static_cast<int>(i);
    }

    // NOTE: Start at end of array.
    iter(array, N);
    std::cout << std::endl;

    return 0;
}
