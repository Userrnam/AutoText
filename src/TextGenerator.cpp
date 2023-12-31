#include "TextGenerator.hpp"

#include <thread>
#include <sstream>
#include <iomanip>

TextGenerator::TextGenerator(int saveRate) {
    memory.rate = saveRate;
}

TextGenerator::~TextGenerator() {
    if (model) {
        running = false;
        // while (status != Status::TextEncoded && status != Status::) {}
        delete model;
        model = nullptr;
    }
}

TextGenerator::Status TextGenerator::getStatus() {
    return status;
}

const char* TextGenerator::getStatusString() {
    return statusToString(status);
}

void TextGenerator::loadModel(const std::string& tokenizerPath, const std::string& modelPath, bool detached) {
    if (running)  return;
    if (detached) {
        auto t = std::thread(&TextGenerator::loadModel, this, tokenizerPath, modelPath, false);
        t.detach();
        return;
    }
    running = true;
    status = Status::LoadingModel;
    if (model) {
        delete model;
        model = nullptr;
    }
    model = new Model(tokenizerPath, modelPath);
    if (!*model) {
        delete model;
        model = nullptr;
        status = Status::ModelLoadingFailed;
    } else {
        status = Status::ModelLoaded;
    }
    running = false;
}

void TextGenerator::encodeText(const std::string& initialText, bool detached) {
    if (running)  return;
    if (initialText.empty()) {
        status = Status::ErrorEmptyInitialText;
        return;
    }

    if (detached) {
        auto t = std::thread(&TextGenerator::encodeText, this, initialText, false);
        t.detach();
        return;
    }

    status = Status::EncodingText;
    running = true;

    auto tokens = model->tokenizer->encode(initialText);

    // find part, shared with model
    int sharedCount = 0;
    for (; sharedCount < model->tokens.size() && sharedCount < tokens.size() &&
           tokens[sharedCount] == model->tokens[sharedCount]; ++sharedCount) {}
    
    int stateIndex = sharedCount / memory.rate - 1;
    if (stateIndex >= 0) {
        model->state = memory.states[stateIndex];
        model->tokens.erase(model->tokens.begin() + (stateIndex + 1) * memory.rate, model->tokens.end());
    } else {
        model->tokens.clear();
    }

    encodingProgress.max = tokens.size();

    auto newStatus = Status::TextEncoded;
    for (size_t i = (stateIndex + 1) * memory.rate; i < tokens.size(); ++i) {
        if (!running) {
            newStatus = Status::TextEncodingInterrupted;
            break;
        }

        model->add(tokens[i]);
        if (model->tokens.size() % memory.rate == 0) {
            memory.states.push_back(model->state);
        }

        encodingProgress.cur = i + 1;
    }

    encodingProgress.max = -1;
    encodingProgress.cur = -1;

    status = newStatus;
    running = false;
}

TextGenerator::TextEncodingProgress TextGenerator::getTextEncodingProgress() {
    return encodingProgress;
}

void TextGenerator::generateText(int count, bool detached) {
    if (running || count == 0)  return;
    if (model->tokens.size() == 0) {
        status = Status::ErrorTextIsNotEncoded;
        return;
    }

    if (detached) {
        auto t = std::thread(&TextGenerator::generateText, this, count, false);
        t.detach();
        return;
    }

    status = Status::GeneratingText;
    running = true;

    auto token = model->sampleDistribution();
    int i = 0;
    while (running) {
        if (i == count)  break;
        {
            std::lock_guard<std::mutex> guard(mtx);
            unreportedTokens.push_back(token);
        }

        model->add(token);
        if (model->tokens.size() % memory.rate == 0) {
            memory.states.push_back(model->state);
        }

        token = model->sampleDistribution();
        i++;
    }

    status = Status::TextGenerationEnded;
    running = false;
}

void TextGenerator::interrupt() {
    running = false;
}

std::string TextGenerator::getGeneratedString() {
    if (unreportedTokens.size() == 0) {
        return "";
    }

    std::lock_guard<std::mutex> guard(mtx);
    auto result = model->tokenizer->decode(unreportedTokens);
    unreportedTokens = {};
    return result;
}

std::string TextGenerator::getReducedDistributionDescription() {
    if (model->tokens.empty()) {
        return "";
    }

    int cur = model->tokens.size();
    if (model->tokens.size() == memory.tokenCountForDescription &&
        memcmp(&memory.samplingParamsForDescription, &model->samplingParams, sizeof(SamplingParameters)) == 0) {
        return memory.distributionDescription;
    }

    std::stringstream ss;

    std::vector<uint32_t> sampleIndices;
    std::vector<float> sampleProbs;
    // FIXME: Possible synchronization Issues
    model->getReducedDistribution(sampleIndices, sampleProbs);
    memory.samplingParamsForDescription = model->samplingParams;

    ss << std::setw(20) << "Token " << std::setw(20) << "Probability\n";
    for (int i = 0; i < sampleIndices.size(); ++i) {
        auto _token = model->decodeToken(sampleIndices[i]);
        std::string token = "\"";
        for (char c : _token) {
            if      (c == ' ')   token += " ";
            else if (c == '\n')  token += "\\n";
            else if (c == '\r')  token += "\\r";
            else if (c == '\t')  token += "\\t";
            else                 token += c;
        }
        token += "\"";

        ss << std::setw(20) << token;
        ss << std::fixed << std::setprecision(4) << std::setw(20) << sampleProbs[i];
        ss << "\n";
    }

    memory.distributionDescription = ss.str();
    memory.tokenCountForDescription = cur;

    return memory.distributionDescription;
}

#define CRS(name) case Status::name: return #name
const char* TextGenerator::statusToString(Status status) {
    switch(status) {
        CRS(None);
        CRS(LoadingModel);
        CRS(ModelLoaded);
        CRS(ModelLoadingFailed);
        CRS(EncodingText);
        CRS(TextEncoded);
        CRS(ErrorEmptyInitialText);
        CRS(TextEncodingInterrupted);
        CRS(GeneratingText);
        CRS(TextGenerationEnded);
        CRS(ErrorTextIsNotEncoded);
    }
}
#undef CRS
