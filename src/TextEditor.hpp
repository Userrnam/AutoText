#pragma once

#include <vector>
#include <string>
#include <imgui.h>
#include <string_view>
#include <tuple>

struct Location {
    size_t paragraphIndex;
    size_t charIndex;
};

struct TextEditor {
    TextEditor();

    bool write(Location& location, const std::string_view& s);
    bool erase(Location start, Location end);
    void append(const std::string& s);

    // returns true if value changed
    bool updateUI(ImVec2 size);

    std::string getString();

    bool editable = true;

private:
    // TODO: vector invokes copy constructor when resizes, use a better data structure
    std::vector<std::string> _text;

    Location _cursor = {};
    Location _selectionStart = {};
    float _cursorAnim = 0;
    bool _wasFocused = false; // required to reset the selectionStart when window gains focus

    bool checkLocation(Location location);
    int handleInput();
};
