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

Model::~Model() {
    delete tokenizer;
    rwkv_free(context);
}

Model::Model(Tokenizer *tokenizer, rwkv_context *context) : tokenizer(tokenizer), context(context) {
    state.resize(rwkv_get_state_buffer_element_count(context));
    logits.resize(rwkv_get_logits_buffer_element_count(context));
}

void Model::add(uint32_t token) {
    float *pState = tokens.size() == 0 ? NULL : &state[0];
    rwkv_eval(context, token, pState, &state[0], &logits[0]);
    tokens.push_back(token);
}

void Model::add(const std::string& s, int* progressCur, int* progressMax) {
    if (s.empty())  return;

    auto ltokens = tokenizer->encode(s);

    if (progressMax) {
        *progressMax = ltokens.size();
    }

    auto start = time(NULL);
    std::cout << "Encoding text" << std::endl;
    for (size_t i = 0; i < ltokens.size(); ++i) {
        add(ltokens[i]);
        if (progressCur) {
            *progressCur = i + 1;
        }
        if (time(NULL) - start == 1) {
            auto end = time(NULL);
            std::cout << i << " / " << ltokens.size() << std::endl;
            start = end;
        }
    }
    if (progressMax)  *progressMax = -1;
    if (progressCur)  *progressCur = -1;
    std::cout << "Text encoded\n";
}

void Model::getReducedDistribution(std::vector<uint32_t>& sampleIndices, std::vector<float>& sampleProbs) {
    auto probs = softmax(logits, samplingParams.temperature);
    float probSum = 0;
    int iters = 0;
    while (probSum < samplingParams.topP && sampleProbs.size() < samplingParams.topK) {
        if (iters == probs.size()) {
            break;
        }

        // find max
        int maxI = 0;
        for (int i = 1; i < probs.size(); ++i) {
            if (probs[maxI] < probs[i]) {
                maxI = i;
            }
        }

        sampleIndices.push_back(maxI);
        sampleProbs.push_back(probs[maxI]);
        probSum += probs[maxI];

        probs[maxI] = 0;
        iters++;
    }
}

uint32_t Model::sampleDistribution() {
    std::vector<uint32_t> sampleIndices;
    std::vector<float> sampleProbs;
    getReducedDistribution(sampleIndices, sampleProbs);

    // TODO:
    // punish repeating
    // int count = std::min((int)tokens.size(), 30);
    // float punishment = 0.9;
    // for (int i = 0; i < count; ++i) {
    //     logits[tokens[tokens.size() - count + i]] *= punishment;
    //     punishment *= 0.9;
    // }

    // zero below avg
    // if (samplingParams.zeroBelowAvg) {
    //     auto average = avg(probs);
    //     for (float& prob : probs) {
    //         if (prob < average) { prob = 0; }
    //     }
    // }

    // sample
    std::random_device rd;
    std::mt19937 gen(rd());
    std::discrete_distribution<> d(sampleProbs.begin(), sampleProbs.end());

    return sampleIndices[d(gen)];
}

std::string Model::decodeToken(uint32_t token) {
    return tokenizer->decode({token});
}
