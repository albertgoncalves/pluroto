#include <cassert>
#include <fstream>
#include <iostream>

int main() {
    std::string path = "./out/read_file.txt";
    {
        std::ofstream file(path);
        assert(file);
        file << "Hello, world!" << std::flush;
        assert(file);
    }
    {
        std::ifstream file(path);
        assert(file);
        std::string message((std::istreambuf_iterator<char>(file)),
                            std::istreambuf_iterator<char>());
        std::cout << message << std::endl;
    }
    return 0;
}
