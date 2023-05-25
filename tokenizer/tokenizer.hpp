#pragma once

#include <vector>
#include <string>
#include <stdint.h>

class Tokenizer {
public:
    Tokenizer(const std::string& file_path);
    ~Tokenizer();

    std::vector<uint32_t> encode(const std::string& s);
    std::string decode(const std::vector<uint32_t>& logits);

    bool ok();

    int buffer_size = 1000;
private:
    struct CTokenizer *m_tokenizer;
};
