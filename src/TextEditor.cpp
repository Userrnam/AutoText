#include "TextEditor.hpp"

#include <iostream>
#include <sstream>
#include <math.h>

TextEditor::TextEditor() {
    _text.push_back("");
}

bool TextEditor::checkLocation(Location location) {
    return location.paragraphIndex < _text.size() && location.charIndex <= _text[location.paragraphIndex].size();
}

bool TextEditor::write(Location location, const std::string_view& s) {
    if (!checkLocation(location)) {
        return false;
    }
    if (s.size() == 0) {
        return true;
    }

    auto end = std::find(s.begin(), s.end(), '\n');
    if (end == s.end()) {
        _text[location.paragraphIndex].insert(_text[location.paragraphIndex].begin() + location.charIndex, s.begin(), end);
        return true;
    }

    // copy everything after the point of insertion
    auto afterInsertion = std::string(_text[location.paragraphIndex].begin() + location.charIndex, _text[location.paragraphIndex].end());

    // remove everything after the insertion
    _text[location.paragraphIndex].erase(_text[location.paragraphIndex].begin() + location.charIndex, _text[location.paragraphIndex].end());

    auto start = s.begin();
    while (true) {
        auto end = std::find(start, s.end(), '\n');
        _text[location.paragraphIndex].insert(_text[location.paragraphIndex].begin() + location.charIndex, start, end);
        if (end != s.end()) {
            _text.insert(_text.begin() + location.paragraphIndex + 1, "");
            location.paragraphIndex++;
            location.charIndex = 0;
            start = end + 1;
        } else {
            // add ending of the first line back
            _text[location.paragraphIndex] += afterInsertion;
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
        auto& paragraph = _text[start.paragraphIndex];
        paragraph.erase(paragraph.begin() + start.charIndex, paragraph.begin() + end.charIndex);
    } else {
        auto& sParagraph = _text[start.paragraphIndex];
        auto& eParagraph = _text[end.paragraphIndex];
        size_t prevSParagraphSize = sParagraph.size();
        sParagraph.erase(sParagraph.begin() + start.charIndex, sParagraph.end());
        sParagraph.insert(sParagraph.begin() + start.charIndex, eParagraph.begin() + end.charIndex, eParagraph.end());
        _text.erase(_text.begin() + start.paragraphIndex + 1, _text.begin() + end.paragraphIndex + 1);
    }

    return true;
}

void TextEditor::append(const std::string& s) {
    assert(write({_text.size()-1, _text.back().size()}, s));
}

char getCharInput() {
    char c = 0;

    bool shift = ImGui::GetIO().KeyShift;

    // letters
    for (int i = 0; i < 26; ++i) {
        if (ImGui::IsKeyPressed(static_cast<ImGuiKey>(ImGuiKey_A + i))) {
            c = 'a' + i;
            break;
        }
    }
    if (c) {
        if (shift) {
            c += ('A' - 'a');
        }
        return c;
    }

    // digits
    for (int i = 0; i < 10; ++i) {
        if (ImGui::IsKeyPressed(static_cast<ImGuiKey>(ImGuiKey_0 + i))) {
            c = '0' + i;
        }
    }
    if (c) {
        if (shift) {
            char arr[] = { ')', '!', '@', '#', '$', '%', '^', '&', '*', '(' };
            return arr[c - '0'];
        }
        return c;
    }
    // other
    if (ImGui::IsKeyPressed(ImGuiKey_Space))  return ' ';
    if (ImGui::IsKeyPressed(ImGuiKey_Enter))  return '\n';

    char arr[][2] = {
        { '\'', '"' },
        { ',', '<' },
        { '-', '_' },
        { '.', '>' },
        { '/', '?' },
        { ';', ':' },
        { '=', '+' },
        { '[', '{' },
        { '\\', '|' },
        { ']', '}' },
        { '`', '~' },
    };
    for (int i = 0; i < 11; ++i) {
        if (ImGui::IsKeyPressed(static_cast<ImGuiKey>(ImGuiKey_Apostrophe + i))) {
            return arr[i][shift];
        }
    }

    return c;
}

// TODO: wrapping aware movement
// 0 - no changes, 1 - changed, 2 - cursor focus, 3 - selection update
int TextEditor::handleInput() {
    char c = getCharInput();

    if (c) {
        if (memcmp(&_cursor, &_selectionStart, sizeof(Location)) != 0) {
            assert(erase(_cursor, _selectionStart));
            _cursor = _selectionStart;
        }
        assert(write(_cursor, std::string_view(&c, 1)));
        if (c == '\n') {
            _cursor.charIndex = 0;
            _cursor.paragraphIndex++;
        } else {
            _cursor.charIndex++;
        }
        return 1;
    }

    if (ImGui::IsKeyPressed(ImGuiKey_Backspace)) {
        if (memcmp(&_cursor, &_selectionStart, sizeof(Location)) != 0) {
            assert(erase(_cursor, _selectionStart));
            _cursor = _selectionStart;
            return 1;
        }

        if (_cursor.paragraphIndex == 0 && _cursor.charIndex == 0) {
            return 2;
        }

        Location newLoc;
        if (_cursor.charIndex == 0) {
            newLoc.paragraphIndex = _cursor.paragraphIndex - 1;
            newLoc.charIndex = _text[newLoc.paragraphIndex].size();
        } else {
            newLoc.paragraphIndex = _cursor.paragraphIndex;
            if (ImGui::GetIO().KeySuper) {
                newLoc.charIndex = 0;
            } else {
                newLoc.charIndex = _cursor.charIndex - 1;
            }
        }

        erase(newLoc, _cursor);

        _cursor = newLoc;

        return 1;
    }

    if (ImGui::IsKeyPressed(ImGuiKey_Delete)) {
        if (memcmp(&_cursor, &_selectionStart, sizeof(Location)) != 0) {
            assert(erase(_cursor, _selectionStart));
            _cursor = _selectionStart;
            return 1;
        }

        if (_cursor.paragraphIndex == _text.size() && _cursor.charIndex == _text.back().size()) {
            return 2;
        }

        Location loc;
        if (_cursor.charIndex == _text[_cursor.paragraphIndex].size()) {
            loc.paragraphIndex = _cursor.paragraphIndex + 1;
            loc.charIndex = 0;
        } else {
            loc.paragraphIndex = _cursor.paragraphIndex;
            if (ImGui::GetIO().KeySuper) {
                loc.charIndex = _text[_cursor.paragraphIndex].size();
            } else {
                loc.charIndex = _cursor.charIndex + 1;
            }
        }

        erase(loc, _cursor);

        return 1;
    }

    if (ImGui::IsKeyPressed(ImGuiKey_UpArrow)) {
        if (_cursor.paragraphIndex != 0) {
            _cursor.paragraphIndex--;
            _cursor.charIndex = 0;
        }
        if (ImGui::GetIO().KeyShift)  return 3;
        return 2;
    }
    if (ImGui::IsKeyPressed(ImGuiKey_DownArrow)) {
        if (_cursor.paragraphIndex != _text.size() - 1) {
            _cursor.paragraphIndex++;
            _cursor.charIndex = 0;
        }
        if (ImGui::GetIO().KeyShift)  return 3;
        return 2;
    }
    if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow)) {
        if (_cursor.charIndex != 0) {
            _cursor.charIndex--;
        }
        if (ImGui::GetIO().KeyShift)  return 3;
        return 2;
    }
    if (ImGui::IsKeyPressed(ImGuiKey_RightArrow)) {
        if (_cursor.charIndex != _text[_cursor.paragraphIndex].size()) {
            _cursor.charIndex++;
        }
        if (ImGui::GetIO().KeyShift)  return 3;
        return 2;
    }

    return 0;
}

bool TextEditor::updateUI(ImVec2 size) {
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImGui::GetColorU32(ImGuiCol_FrameBg));
    ImGui::BeginChild("Scrolling", size, true, ImGuiWindowFlags_NoNavInputs);
    bool focused = ImGui::IsWindowFocused();

    int inputStatus = 0;
    if (focused && editable) {
        inputStatus = handleInput(); 
    }

    if (inputStatus && inputStatus != 3) {
        _selectionStart = _cursor;
    }

    auto windowPos = ImGui::GetCursorPos();
    auto windowSize = ImGui::GetContentRegionAvail();
    int lineWidthMax = windowSize.x / ImGui::CalcTextSize("W").x;
    if (ImGui::IsWindowHovered())
        ImGui::SetMouseCursor(ImGuiMouseCursor_TextInput);

    std::vector<int> linesBeginnings;
    // Display and handle mouse click
    for (int paragraphIndex = 0; paragraphIndex < _text.size(); ++paragraphIndex) {
        linesBeginnings = {0};
        auto paragraphCopy = _text[paragraphIndex];
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
            linesBeginnings.push_back(_text[paragraphIndex].size() + 1);
        }
        ImGui::Text("%s", paragraphCopy.c_str());
        auto lineHeight = ImGui::GetTextLineHeight();
        // update cursor position
        if (ImGui::IsMouseDown(0)) {
            ImVec2 itemMin = ImGui::GetItemRectMin();
            ImVec2 itemMax = ImVec2(itemMin.x + windowSize.x, ImGui::GetItemRectMax().y);
            if (focused && ImGui::IsMouseHoveringRect(itemMin, itemMax)) {
                // find on which character user clicked.
                auto mousePos = ImGui::GetMousePos();

                // coordinates inside the text
                mousePos.x -= itemMin.x;
                mousePos.y -= itemMin.y;

                int lineNumber = (mousePos.y - 0.01f) / lineHeight;
                assert(lineNumber < linesBeginnings.size());

                bool found = false;
                auto start = &_text[paragraphIndex][linesBeginnings[lineNumber]];
                int searchStartIndex = (mousePos.x - 0.01f) / ImGui::CalcTextSize(".").x;
                for (int i = linesBeginnings[lineNumber] + 1; i < linesBeginnings[lineNumber+1]; ++i) {
                    auto textSize = ImGui::CalcTextSize(start, &_text[paragraphIndex][i]);
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

            if (!_wasFocused || ImGui::IsMouseClicked(0)) {
                _selectionStart = _cursor;
            }
        }

        // render selection
        if (memcmp(&_cursor, &_selectionStart, sizeof(Location)) != 0) {
            auto start = _cursor;
            auto end = _selectionStart;
            if (end.paragraphIndex < start.paragraphIndex ||
                (start.paragraphIndex == end.paragraphIndex && end.charIndex < start.charIndex)) {
                std::swap(start, end);
            }

            ImDrawList* drawList = ImGui::GetWindowDrawList();
            ImU32 bgColor = ImGui::GetColorU32(ImGuiCol_TextSelectedBg);
            auto& paragraph = _text[paragraphIndex];
            float minSelectionWidth = 5.0f;

            if (start.paragraphIndex <= paragraphIndex && paragraphIndex <= end.paragraphIndex) {
                auto topLeft = ImGui::GetItemRectMin();
                bool startFound = false;
                bool endFound = false;
                for (int i = 1; i < linesBeginnings.size() && !endFound; ++i) {
                    float selectionBegin = 0;
                    if (start.paragraphIndex == paragraphIndex && !startFound) {
                        if (start.charIndex < linesBeginnings[i]) {
                            selectionBegin = ImGui::CalcTextSize(&paragraph[linesBeginnings[i-1]], &paragraph[start.charIndex]).x;
                            startFound = true;
                        } else {
                            continue;
                        }
                    }
                    float selectionEnd;
                    if (end.paragraphIndex == paragraphIndex && end.charIndex < linesBeginnings[i]) {
                        selectionEnd = ImGui::CalcTextSize(&paragraph[linesBeginnings[i-1]], &paragraph[end.charIndex]).x;
                        endFound = true;
                    } else {
                        selectionEnd = ImGui::CalcTextSize(&paragraph[linesBeginnings[i-1]], &paragraph[linesBeginnings[i]-1]).x;
                    }
                    auto p1 = ImVec2(topLeft.x + selectionBegin, topLeft.y + lineHeight * (i-1));
                    auto p2 = ImVec2(topLeft.x + selectionEnd, topLeft.y + lineHeight * i);
                    if (p2.x - p1.x < minSelectionWidth) {
                        p2.x = p1.x + minSelectionWidth;
                    }
                    drawList->AddRectFilled(p1, p2, bgColor);
                }
            }
        }

        if (!focused)  continue;

        // render cursor
        if (_cursor.paragraphIndex == paragraphIndex) {
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
            cursorPos.x += ImGui::CalcTextSize(&_text[paragraphIndex][linesBeginnings[lineNumber]],
                                               &_text[paragraphIndex][_cursor.charIndex]).x;

            if (inputStatus) {
                float scrollY = ImGui::GetScrollY();
                if (cursorPos.y < windowPos.y) {
                    scrollY -= windowPos.y - cursorPos.y;
                    ImGui::SetScrollY(scrollY);
                } else if (cursorPos.y > windowPos.y + windowSize.y) {
                    scrollY += cursorPos.y - (windowPos.y + windowSize.y) + 5;
                    ImGui::SetScrollY(scrollY);
                }
            }

            if (windowPos.y - lineHeight < cursorPos.y && cursorPos.y - lineHeight < windowPos.y + windowSize.y) {
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
    }

    _wasFocused = focused;

    ImGui::EndChild();
    ImGui::PopStyleColor();

    ImGui::Text("%s", focused ? "Focused" : "Not Focused");

    return inputStatus == 1;
}

std::string TextEditor::getString() {
    std::stringstream ss;
    for (int i = 0; i < _text.size(); ++i) {
        if (i != 0) {
            ss << '\n';
        }
        ss << _text[i];
    }
    return ss.str();
}
