#include "tokenizer.hpp"


extern "C" {

typedef struct CTokenizer CTokenizer;

CTokenizer* tokenizer_from_file(const char* file_path);
void free_tokenizer(CTokenizer* tokenizer);

int32_t tokenizer_encode(CTokenizer* tokenizer,
                         const char* str,
                         uint32_t* result_data,
                         uint32_t  result_allocated);

int32_t tokenizer_decode(CTokenizer* tokenizer,
                         const uint32_t* input_data,
                         uint32_t  input_count,
                         char*     result_data,
                         uint32_t  result_allocated);
}

Tokenizer::Tokenizer(const std::string& file_path) {
    m_tokenizer = tokenizer_from_file(file_path.c_str());
}

Tokenizer::~Tokenizer() {
    free_tokenizer(m_tokenizer);
}

bool Tokenizer::ok() {
    return m_tokenizer != nullptr;
}

std::vector<uint32_t> Tokenizer::encode(const std::string& s) {
    std::vector<uint32_t> result(buffer_size);
    int count = tokenizer_encode(m_tokenizer, s.c_str(), &result[0], result.size());
    result.resize(count);
    if (count > buffer_size) {
        tokenizer_encode(m_tokenizer, s.c_str(), &result[0], result.size());
    }
    return result;
}

std::string Tokenizer::decode(const std::vector<uint32_t>& logits) {
    std::string result(buffer_size, ' ');
    int count = tokenizer_decode(m_tokenizer, &logits[0], logits.size(), &result[0], result.size());
    result.resize(count);
    if (count > buffer_size) {
        tokenizer_decode(m_tokenizer, &logits[0], logits.size(), &result[0], result.size());
    }
    return result;
}
