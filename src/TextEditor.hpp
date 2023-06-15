#pragma once

#include <vector>
#include <string>
#include <imgui.h>

struct Location {
    size_t paragraphIndex;
    size_t charIndex;
};

struct TextEditor {
    // TODO: vector invokes copy constructor when resizes, use a better data structure
    std::vector<std::string> text;

    TextEditor();

    bool write(Location location, const std::string& s);
    bool erase(Location start, Location end);
    void append(const std::string& s);

    // returns true if value changed
    bool updateUI(ImVec2 size);

private:
    Location _cursor;
    float _cursorAnim = 0;

    bool checkLocation(Location location);
};
