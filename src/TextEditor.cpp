#include "TextEditor.hpp"

#include <iostream>

TextEditor::TextEditor() {
    // text.push_back("");
    for (int i = 0; i < 50; ++i) {
        text.push_back(std::to_string(i) + ": this is some text asfoia iweofja wefjaowei foawejf ioawjeoifj ao");
    }
}

bool TextEditor::checkLocation(Location location) {
    return location.line < text.size() && location.index <= text[location.line].size();
}

bool TextEditor::write(Location location, const std::string& s) {
    if (!checkLocation(location)) {
        return false;
    }
    if (s.size() == 0) {
        return true;
    }

    auto start = s.begin();
    while (true) {
        auto end = std::find(start, s.end(), '\n');
        text[location.line].insert(text[location.line].begin(), start, end);
        if (end != s.end()) {
            text.insert(text.begin() + location.line + 1, "");
            start = end + 1;
        } else {
            break;
        }
    };

    return true;
}

bool TextEditor::erase(Location start, Location end) {
    if (!checkLocation(start) || !checkLocation(end)) {
        return false;
    }

    if (end.line < start.line || (start.line == end.line && end.index < start.index)) {
        std::swap(start, end);
    }

    if (start.line == end.line) {
        auto& line = text[start.line];
        line.erase(line.begin() + start.index, line.begin() + end.index);
    } else {
        auto& sLine = text[start.line];
        auto& eLine = text[end.line];
        size_t prevSLineSize = sLine.size();
        sLine.insert(sLine.begin() + start.index, eLine.begin() + end.index, eLine.end());
        if (prevSLineSize > sLine.size()) {
            sLine.erase(sLine.begin() + start.index + eLine.size() - end.index, sLine.end());
        }
        text.erase(text.begin() + start.line + 1, text.begin() + end.line + 1);
    }

    return true;
}

void TextEditor::append(const std::string& s) {
    write({text.size()-1, text.back().size()}, s);
}

bool TextEditor::updateUI(ImVec2 size) {
    auto cursor = ImGui::GetCursorPos();

    ImGui::BeginChild("Scrolling", size);
    _focused = ImGui::IsWindowFocused();

    auto windowWidth = ImGui::GetWindowContentRegionWidth();
    int lineWidthMax = windowWidth / ImGui::CalcTextSize("W").x;

    std::vector<int> linesBeginnings;
    for (int line = 0; line < text.size(); ++line) {
        linesBeginnings = {0};
        auto lineCopy = text[line];
        // calc wrap locations
        {
            int lineWidth = 0;
            int wrapLocation = 0;
            for (int i = 0; i < lineCopy.size() && lineCopy[i]; ++i) {
                if (i < lineCopy.size()-1 && std::isspace(lineCopy[i+1])) {
                    wrapLocation = i+1;
                }
                if (lineWidth == lineWidthMax) {
                    // if the word takes up more than lineWidthMax, go till it's end
                    if (i - wrapLocation >= lineWidthMax) {
                        for (; i < lineCopy.size() && std::isspace(lineCopy[i]); ++i) {}
                        wrapLocation = i;
                    }
                    lineCopy[wrapLocation] = '\n';
                    linesBeginnings.push_back(wrapLocation+1);
                    i = wrapLocation;
                    lineWidth = 0;
                    continue;
                }
                lineWidth++;
            }
            linesBeginnings.push_back(text[line].size() + 1);
        }
        ImGui::Text("%s", lineCopy.c_str());
        if (ImGui::IsMouseClicked(0)) {
            ImVec2 itemMin = ImGui::GetItemRectMin();
            ImVec2 itemMax = ImVec2(itemMin.x + windowWidth, ImGui::GetItemRectMax().y);
            auto lineHeight = ImGui::GetTextLineHeight();
            if (ImGui::IsMouseHoveringRect(itemMin, itemMax)) {
                // find on which character user clicked.
                auto mousePos = ImGui::GetMousePos();

                // coordinates inside the text
                mousePos.x -= itemMin.x;
                mousePos.y -= itemMin.y;

                int lineNumber = (mousePos.y - 0.01f) / lineHeight;
                assert(lineNumber < linesBeginnings.size());

                bool found = false;
                auto start = &text[line][linesBeginnings[lineNumber]];
                int searchStartIndex = (mousePos.x - 0.01f) / ImGui::CalcTextSize(".").x;
                for (int i = linesBeginnings[lineNumber] + 1; i < linesBeginnings[lineNumber+1]; ++i) {
                    auto textSize = ImGui::CalcTextSize(start, &text[line][i]);
                    if (mousePos.x < textSize.x) {
                        found = true;
                        std::cout << "SL: " << lineNumber << ", Char: " << text[line][i-1] << std::endl;
                        break;
                    }
                }
                if (!found) {
                    std::cout << "End of SL: " << lineNumber << std::endl;
                }

                std::cout << "Clicked " << line << std::endl;
            }
        }
    }
    ImGui::EndChild();
    ImGui::Text("%s", _focused ? "Focused" : "Not Focused");

    // ImVec2 mousePos = ImGui::GetMousePos();
    // ImVec2 charPos = ImGui::GetCursorScreenPos();
    // const char* text = "Hello World";
    // float charWidth = ImGui::CalcTextSize(" ").x; // Assuming each character has the same width

    // for (int i = 0; i < strlen(text); i++)
    // {
    //     float charHeight = ImGui::GetTextLineHeight();

    //     ImGui::SameLine();
    //     // Check if the mouse is within the bounding box of the character
    //     if (mousePos.x >= charPos.x && mousePos.x <= (charPos.x + charWidth) &&
    //         mousePos.y >= charPos.y && mousePos.y <= (charPos.y + charHeight))
    //     {
    //         // The mouse is above this character
    //         ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "%c", text[i]);
    //     }
    //     else
    //     {
    //         // The mouse is not above this character
    //         ImGui::Text("%c", text[i]);
    //     }
    //     charPos.x += charWidth;
    // }

    return true;
}
