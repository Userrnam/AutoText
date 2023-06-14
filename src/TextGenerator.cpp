#include "TextGenerator.hpp"

#include <thread>

TextGenerator::TextGenerator() {}
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
    model = new Model(tokenizerPath, modelPath);
    if (!*model) {
        delete model;
        model = nullptr;
        status = Status::ModelLoadingFailed;
    }
    status = Status::ModelLoaded;
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
    encodingProgress.max = tokens.size();

    auto newStatus = Status::TextEncoded;
    for (size_t i = 0; i < tokens.size(); ++i) {
        if (!running) {
            newStatus = Status::TextEncodingInterrupted;
            model->tokens.clear();
            break;
        }

        model->add(tokens[i]);
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
