#include <algorithm>
#include <array>
#include <iostream>
#include <tuple>

template <typename A, typename B>
static std::ostream& operator<<(std::ostream& stream, std::tuple<A, B>& tuple) {
    stream << '(' << std::get<0>(tuple) << ", " << std::get<1>(tuple) << ')';
    return stream;
}

template <typename T, size_t N>
static std::ostream& operator<<(std::ostream& stream, std::array<T, N>& array) {
    stream << '[';
    for (size_t i = 0; i < (N - 1); ++i) {
        stream << array[i] << ", ";
    }
    stream << array[N - 1] << ']';
    return stream;
}

int main() {
    std::array<std::tuple<int, int>, 4> items{
        std::tuple<int, int>(2, -1),
        std::tuple<int, int>(1, 0),
        std::tuple<int, int>(-2, 123),
        std::tuple<int, int>(1, 2),
    };

    std::sort(items.begin(), items.end(), [](std::tuple<int, int> l, std::tuple<int, int> r) {
        if (std::get<1>(l) == std::get<1>(r)) {
            return std::get<0>(l) < std::get<0>(r);
        }
        return std::get<1>(l) < std::get<1>(r);
    });

    std::cout << items << '\n';

    return 0;
}
