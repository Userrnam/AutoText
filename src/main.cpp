#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>

#include "Model.hpp"

template<typename T>
std::ostream& operator<<(std::ostream& os, const std::vector<T>& v) {
    os << "[ ";
    for (const auto& e : v) {
        os << e << " ";
    }
    os << "]";
    return os;
}

std::string read_file(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << path << std::endl;
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();

    return buffer.str();
}

int main() {
    std::string path = "/Users/antonkondratuk/LM/rwkv.cpp/";
    Model model(
        path + "rwkv/20B_tokenizer.json",
        path + "Q8_0-RWKV-4-Raven-1B5-v9-20230411-ctx4096.bin");
    
    if (!model) {
        return -1;
    }
    
    std::string text = read_file(path + "input1.txt");

    model.encodeText(text);

    std::cout << "T: " << text << std::endl;
    auto state = model.state;
    auto nt = model.nextToken;
    while (true) {
        for (int i = 0; i < 50; ++i) {
            std::cout << model.next() << std::flush;
        }
        std::cout << std::endl;

        std::string cmd;
        std::cin >> cmd;

        if (cmd == "c") {
            state = model.state;
            nt = model.nextToken;
        } else if (cmd == "r") {
            model.state = state;
            model.nextToken = nt;
        } else {
            break;
        }
    }

    return 0;
}
