#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>

#include "Model.hpp"
#include "Application.hpp"

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
    Application app("AutoText");
    app.run();

    return 0;

    // std::string text = "Well, Prince, so Genoa and Lucca are now just family estates of the Buonapartes. But I warn you, if you don’t tell me that this means war, if you still try to defend the infamies and horrors ";

    // Model model(std::string(PROJECT_PATH) + "/rwkv.cpp/rwkv/20B_tokenizer.json",
    //             std::string(PROJECT_PATH) + "/models/Q8_0-RWKV-4-Raven-1B5-v9-20230411-ctx4096.bin");
    
    // if (!model) {
    //     return 1;
    // }

    // model.add(text);

    // std::cout << "T: " << text << std::endl;
    // for (int i = 0; i < 50; ++i) {
    //     auto token = model.sampleDistribution();
    //     std::cout << model.decodeToken(token) << std::flush;
    //     model.add(token);
    // }

    // return 0;
}
