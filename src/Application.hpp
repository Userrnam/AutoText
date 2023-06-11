#pragma once

#include <vector>
#include <string>
#include <thread>

#include "Model.hpp"

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

    Model *model = nullptr;
    int progressCur = -1;
    int progressMax = -1;

    bool modelLoaded = false;
    std::thread *loadingThread = nullptr;
    std::thread *textGenerationThread = nullptr;
};
