#pragma once

#include <vector>
#include <cstdint>
#include <rwkv.h>
#include <tokenizer.hpp>


struct SamplingParameters {
    float temperature = 0.5;
    int topK = 50;
    bool zeroBelowAvg = true;
};

struct Model {
    Tokenizer *tokenizer;
    rwkv_context *context;
    SamplingParameters samplingParams;

    std::vector<float> state;
    std::vector<float> logits;
    std::vector<uint32_t> tokens;
    uint32_t nextToken;

    Model(const std::string& tokenizerPath, const std::string& modelPath);
    Model(Tokenizer *tokenizer, rwkv_context *context);

    operator bool() { return context != nullptr && tokenizer->ok(); }

    void encodeText(const std::string& text);
    std::string next();
};
