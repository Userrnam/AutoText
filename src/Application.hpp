#pragma once

#include <vector>
#include <string>
#include <thread>

#include "TextGenerator.hpp"


// TODO: ensure text cannot be modified during encoding.
struct Application {
    Application(const char *name);
    ~Application();

    void run();

    std::vector<char> text;
    size_t textSize = 0;

    std::string appPath = PROJECT_PATH;
    std::string modelName;

    void updateUI();
    void updateText();
    void showFileExplorer(bool *p_open);
    void loadModel();

    bool stopGeneration = false;
    bool joinTextGenerationThread = false;
    void textGeneration();

    struct GLFWwindow* window = nullptr;

    TextGenerator textGenerator;
};
