#include "Model.hpp"

#include <random>
#include <iostream>

uint32_t mostLikely(const std::vector<float>& logits) {
    uint32_t result = 0;
    for (size_t i = 0; i < logits.size(); ++i) {
        if (logits[i] > logits[result]) {
            result = i;
        }
    }
    return result;
}

void softmax(std::vector<float>& logits, float temperature = 1.0f) {
    float sum = 0;
    for (auto& logit : logits) {
        logit = exp(logit / temperature);
        sum += logit;
    }
    for (auto& logit : logits) {
        logit /= sum;
    }
}

template<typename T>
float avg(const std::vector<T>& v) {
    T result = v[0];
    for (size_t i = 1; i < v.size(); ++i) {
        result += v[i];
    }
    return result / v.size();
}

Model::Model(const std::string& tokenizerPath, const std::string& modelPath) {
    tokenizer = new Tokenizer(tokenizerPath);
    if (!tokenizer->ok()) {
        std::cout << "Failed to load tokenizer\n";
    }
    context = rwkv_init_from_file(modelPath.c_str(), 5);
    if (context) {
        std::cout << "Model loaded\n";
    } else {
        std::cout << "Failed to load model\n";
    }
    state.resize(rwkv_get_state_buffer_element_count(context));
    logits.resize(rwkv_get_logits_buffer_element_count(context));
}

Model::Model(Tokenizer *tokenizer, rwkv_context *context) : tokenizer(tokenizer), context(context) {
    state.resize(rwkv_get_state_buffer_element_count(context));
    logits.resize(rwkv_get_logits_buffer_element_count(context));
}

void Model::encodeText(const std::string& text) {
    tokens = tokenizer->encode(text);

    auto start = time(NULL);
    std::cout << "Encoding text" << std::endl;
    for (size_t i = 0; i < tokens.size() - 1; ++i) {
        if (i == 0) {
            rwkv_eval(context, tokens[i], NULL, &state[0], &logits[0]);
        } else {
            rwkv_eval(context, tokens[i], &state[0], &state[0], &logits[0]);
        }
        if (time(NULL) - start == 1) {
            auto end = time(NULL);
            std::cout << i << " / " << tokens.size() << std::endl;
            start = end;
        }
    }
    nextToken = tokens.back();

    std::cout << "Text encoded\n";
}

std::string Model::next() {
    rwkv_eval(context, nextToken, &state[0], &state[0], &logits[0]);

    softmax(logits, samplingParams.temperature);

    // punish repeating
    int count = std::min((int)tokens.size(), 30);
    float punishment = 0.9;
    for (int i = 0; i < count; ++i) {
        logits[tokens[tokens.size() - count + i]] *= punishment;
        punishment *= 0.9;
    }

    // zero below avg
    if (samplingParams.zeroBelowAvg) {
        auto average = avg(logits);
        for (float& logit : logits) {
            if (logit < average) { logit = 0; }
        }
    }

    // sample
    std::random_device rd;
    std::mt19937 gen(rd());
    std::discrete_distribution<> d(logits.begin(), logits.end());

    nextToken = d(gen);

    tokens.push_back(nextToken);

    return tokenizer->decode({nextToken});
}