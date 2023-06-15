#include "TextEditor.hpp"

#include <iostream>
#include <math.h>

TextEditor::TextEditor() {
    // text.push_back("");
    for (int i = 0; i < 50; ++i) {
        text.push_back(std::to_string(i) + ": this is some text asfoia iweofja wefjaowei foawejf ioawjeoifj ao");
    }
}

bool TextEditor::checkLocation(Location location) {
    return location.paragraphIndex < text.size() && location.charIndex <= text[location.paragraphIndex].size();
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
        text[location.paragraphIndex].insert(text[location.paragraphIndex].begin(), start, end);
        if (end != s.end()) {
            text.insert(text.begin() + location.paragraphIndex + 1, "");
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

    if (end.paragraphIndex < start.paragraphIndex ||
       (start.paragraphIndex == end.paragraphIndex && end.charIndex < start.charIndex)) {
        std::swap(start, end);
    }

    if (start.paragraphIndex == end.paragraphIndex) {
        auto& paragraph = text[start.paragraphIndex];
        paragraph.erase(paragraph.begin() + start.charIndex, paragraph.begin() + end.charIndex);
    } else {
        auto& sParagraph = text[start.paragraphIndex];
        auto& eParagraph = text[end.paragraphIndex];
        size_t prevSParagraphSize = sParagraph.size();
        sParagraph.insert(sParagraph.begin() + start.charIndex, eParagraph.begin() + end.charIndex, eParagraph.end());
        if (prevSParagraphSize > sParagraph.size()) {
            sParagraph.erase(sParagraph.begin() + start.charIndex + eParagraph.size() - end.charIndex, sParagraph.end());
        }
        text.erase(text.begin() + start.paragraphIndex + 1, text.begin() + end.paragraphIndex + 1);
    }

    return true;
}

void TextEditor::append(const std::string& s) {
    write({text.size()-1, text.back().size()}, s);
}

bool TextEditor::updateUI(ImVec2 size) {
    auto cursor = ImGui::GetCursorPos();

    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetColorU32(ImGuiCol_FrameBg));
    ImGui::BeginChild("Scrolling", size);
    bool focused = ImGui::IsWindowFocused();

    auto windowWidth = ImGui::GetWindowContentRegionWidth();
    int lineWidthMax = windowWidth / ImGui::CalcTextSize("W").x;

    std::vector<int> linesBeginnings;
    // Display and handle mouse click
    for (int paragraphIndex = 0; paragraphIndex < text.size(); ++paragraphIndex) {
        linesBeginnings = {0};
        auto paragraphCopy = text[paragraphIndex];
        // calc wrap locations
        {
            int lineWidth = 0;
            int wrapLocation = 0;
            for (int i = 0; i < paragraphCopy.size() && paragraphCopy[i]; ++i) {
                if (i < paragraphCopy.size()-1 && std::isspace(paragraphCopy[i+1])) {
                    wrapLocation = i+1;
                }
                if (lineWidth == lineWidthMax) {
                    // if the word takes up more than lineWidthMax, go till it's end
                    if (i - wrapLocation >= lineWidthMax) {
                        for (; i < paragraphCopy.size() && std::isspace(paragraphCopy[i]); ++i) {}
                        wrapLocation = i;
                    }
                    paragraphCopy[wrapLocation] = '\n';
                    linesBeginnings.push_back(wrapLocation+1);
                    i = wrapLocation;
                    lineWidth = 0;
                    continue;
                }
                lineWidth++;
            }
            linesBeginnings.push_back(text[paragraphIndex].size() + 1);
        }
        ImGui::Text("%s", paragraphCopy.c_str());
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
                auto start = &text[paragraphIndex][linesBeginnings[lineNumber]];
                int searchStartIndex = (mousePos.x - 0.01f) / ImGui::CalcTextSize(".").x;
                for (int i = linesBeginnings[lineNumber] + 1; i < linesBeginnings[lineNumber+1]; ++i) {
                    auto textSize = ImGui::CalcTextSize(start, &text[paragraphIndex][i]);
                    if (mousePos.x < textSize.x) {
                        found = true;
                        _cursor.paragraphIndex = paragraphIndex;
                        _cursor.charIndex = i-1;
                        break;
                    }
                }
                if (!found) {
                    _cursor.paragraphIndex = paragraphIndex;
                    _cursor.charIndex = linesBeginnings[lineNumber + 1] - 1;
                }
            }
        }
        if (!focused)  continue;

        // render cursor
        if (ImGui::IsItemVisible() && _cursor.paragraphIndex == paragraphIndex) {
            // get cursor line
            int lineNumber = 0;
            for (int i = 1; i < linesBeginnings.size(); ++i) {
                if (_cursor.charIndex < linesBeginnings[i]) {
                    lineNumber = i - 1;
                    break;
                }
            }

            ImVec2 cursorPos = ImGui::GetItemRectMin();
            auto lineHeight = ImGui::GetTextLineHeight();

            cursorPos.y += lineHeight * lineNumber;
            cursorPos.x += ImGui::CalcTextSize(&text[paragraphIndex][linesBeginnings[lineNumber]],
                                               &text[paragraphIndex][_cursor.charIndex]).x;

            _cursorAnim += ImGui::GetIO().DeltaTime;
            bool cursorIsVisible = (!ImGui::GetIO().ConfigInputTextCursorBlink) || (_cursorAnim <= 0.0f) || fmodf(_cursorAnim, 1.20f) <= 0.80f;
            if (cursorIsVisible) {
                ImDrawList* drawList = ImGui::GetWindowDrawList();
                drawList->AddLine(ImVec2(cursorPos.x, cursorPos.y + 0.5f),
                                ImVec2(cursorPos.x, cursorPos.y + ImGui::GetFontSize() - 0.5f),
                                ImGui::GetColorU32(ImGuiCol_Text));
            }
        }
    }
    ImGui::EndChild();
    ImGui::PopStyleColor();

    ImGui::Text("%s", focused ? "Focused" : "Not Focused");

    return true;
}
