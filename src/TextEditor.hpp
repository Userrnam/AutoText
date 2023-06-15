#pragma once

#include <vector>
#include <string>
#include <imgui.h>

struct Location {
    size_t line;
    size_t index;
};

struct TextEditor {
    // TODO: vector invokes copy constructor when resizes, use a better data structure
    std::vector<std::string> text;

    TextEditor();

    bool write(Location location, const std::string& s);
    bool erase(Location start, Location end);
    void append(const std::string& s);

    bool updateUI(ImVec2 size);

private:
    bool _focused = false;
    bool checkLocation(Location location);
};
