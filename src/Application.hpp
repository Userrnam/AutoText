#pragma once

struct Application {
    Application(const char *name);
    ~Application();

    void run();

    struct GLFWwindow* window;
};
