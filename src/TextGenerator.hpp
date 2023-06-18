#pragma once

#include "Model.hpp"

#include <mutex>

struct TextGenerator {
    enum class Status {
        None,
        LoadingModel, ModelLoaded, ModelLoadingFailed,
        EncodingText, TextEncoded, ErrorEmptyInitialText, TextEncodingInterrupted,
        GeneratingText, TextGenerationEnded, ErrorTextIsNotEncoded,
    };

    struct TextEncodingProgress {
        int cur = -1;
        int max = -1;
    };

    TextGenerator(int saveRate = 100);
    ~TextGenerator();

    Status getStatus();
    static const char* statusToString(Status status);
    const char* getStatusString();

    void loadModel(const std::string& tokenizerPath, const std::string& modelPath, bool detached = true);
    void encodeText(const std::string& initialText, bool detached = true);
    TextEncodingProgress getTextEncodingProgress();
    void generateText(int count = -1, bool detached = true); // -1 means text will be generated unti interrupted

    void interrupt();
    bool isRunning() { return running; }

    bool modelLoaded() { return model != nullptr; }

    std::string getGeneratedString();

private:
    Model *model = nullptr;
    Status status = Status::None;

    struct {
        std::vector<std::vector<float>> states;
        int rate  = 100; // saves state after each 100 tokens

        int TODOstart = 0;   // token location of the first state in memory
        int TODOmax   = 5;   // max number of states
    } memory;
    std::vector<uint32_t> unreportedTokens;
    std::mutex mtx;

    TextEncodingProgress encodingProgress;

    bool running = false; // has a running thread
};
