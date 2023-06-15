#pragma once

#include <vector>
#include <string>
#include <thread>

#include "TextGenerator.hpp"
#include "TextEditor.hpp"

// TODO: ensure text cannot be modified during encoding.
struct Application {
    Application(const char *name);
    ~Application();

    void run();

    void updateUI();
    void updateText();
    void showFileExplorer(bool *p_open);
    void loadModel();

    struct GLFWwindow* window = nullptr;

    TextEditor textEditor;

    TextGenerator textGenerator;

    std::vector<char> text;
    size_t textSize = 0;

    std::string appPath = PROJECT_PATH;
    std::string modelName;
};
