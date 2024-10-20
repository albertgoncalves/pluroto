#include <iostream>
#include <string_view>
#include <vector>

template <typename T>
static std::ostream& operator<<(std::ostream& stream, std::vector<T>& vec) {
    const size_t n = vec.size();
    if (n == 0) {
        stream << "[]";
        return stream;
    }

    stream << '[' << vec[0];
    for (size_t i = 1; i < n; ++i) {
        stream << ", " << vec[i];
    }
    stream << ']';
    return stream;
}

int main() {
    const char* string = " \nsome string\twith\nspaces\r\n";
    const char* spaces = " \t\n\r";

    const std::string_view sv(string);

    std::vector<std::string_view> tokens;

    const size_t n = sv.size();
    size_t       i = 0;
    for (;;) {
        size_t j = sv.find_first_not_of(spaces, i);
        if (n <= j) {
            break;
        }
        i = j;
        j = sv.find_first_of(spaces, i);

        std::string_view token = sv.substr(i, j - i);
        tokens.push_back(token);
        std::cout << i << ", " << j << std::endl;

        if (n <= j) {
            break;
        }
        i = j;
    }

    std::cout << tokens << std::endl;

    return 0;
}
