#include "Model.hpp"

#include <random>
#include <iostream>


std::vector<float> softmax(const std::vector<float>& logits, float temperature = 1.0f) {
    std::vector<float> result(logits.size());
    float sum = 0;
    for (size_t i = 0; i < logits.size(); ++i) {
        result[i] = exp(logits[i] / temperature);
        sum += result[i];
    }
    for (size_t i = 0; i < result.size(); ++i) {
        result[i] /= sum;
    }
    return result;
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
        return;
    }
    context = rwkv_init_from_file(modelPath.c_str(), 5);
    if (context) {
        std::cout << "Model loaded\n";
    } else {
        std::cout << "Failed to load model\n";
        return;
    }
    state.resize(rwkv_get_state_buffer_element_count(context));
    logits.resize(rwkv_get_logits_buffer_element_count(context));
}

Model::Model(Tokenizer *tokenizer, rwkv_context *context) : tokenizer(tokenizer), context(context) {
    state.resize(rwkv_get_state_buffer_element_count(context));
    logits.resize(rwkv_get_logits_buffer_element_count(context));
}

void Model::add(uint32_t token) {
    tokens.push_back(token);
    rwkv_eval(context, token, &state[0], &state[0], &logits[0]);
}

void Model::add(const std::string& s) {
    if (s.empty())  return;

    auto ltokens = tokenizer->encode(s);
    for (auto token : ltokens) {
        tokens.push_back(token);
    }

    auto start = time(NULL);
    std::cout << "Encoding text" << std::endl;
    for (size_t i = 0; i < ltokens.size(); ++i) {
        if (i == 0) {
            rwkv_eval(context, ltokens[i], NULL, &state[0], &logits[0]);
        } else {
            rwkv_eval(context, ltokens[i], &state[0], &state[0], &logits[0]);
        }
        if (time(NULL) - start == 1) {
            auto end = time(NULL);
            std::cout << i << " / " << ltokens.size() << std::endl;
            start = end;
        }
    }
    std::cout << "Text encoded\n";
}

uint32_t Model::sampleDistribution() {
    auto probs = softmax(logits, samplingParams.temperature);

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

    return d(gen);
}

std::string Model::decodeToken(uint32_t token) {
    return tokenizer->decode({token});
}
