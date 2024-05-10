#include <cassert>
#include <fstream>
#include <iostream>

int main() {
    std::string path = "./out/read_file.txt";
    {
        std::ofstream file(path, std::ios::out);
        assert(file);
        file << "Hello, world!";
    }
    {
        std::ifstream file(path, std::ios::in);
        assert(file);
        std::string message((std::istreambuf_iterator<char>(file)),
                            std::istreambuf_iterator<char>());
        std::cout << message << '\n';
    }

    return 0;
}
