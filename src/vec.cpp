#include <iostream>
#include <vector>

template <typename T>
static std::ostream& operator<<(std::ostream& stream, std::vector<T>& vec) {
    size_t n = vec.size() - 1u;

    stream << '[';
    for (size_t i = 0; i < n; ++i) {
        stream << vec[i] << ", ";
    }
    stream << vec[n] << ']';
    return stream;
}

int main() {
    std::vector<int> vec = {};
    vec.push_back(-123);
    vec.push_back(2);
    vec.push_back(3456);
    vec[1] = -2;

    std::cout << vec << std::endl;
}
