#pragma once

#include <vector>
#include <cstdint>
#include <string>
#include "basicui/basicui.h"

namespace bui {

struct Cell {
    char32_t codepoint = ' ';
    Color fg = MakeColor(204, 204, 204, 255); // #CCCCCC
    Color bg = MakeColor(12, 12, 12, 255);    // #0C0C0C
    uint8_t flags = 0;
    uint16_t font = 0; // index into TextGrid font table; 0 = default

    enum Flags : uint8_t {
        Bold = 1 << 0,
        Italic = 1 << 1,
        Underline = 1 << 2,
        Blink = 1 << 3,
        Reverse = 1 << 4,
    };
};

class TextGrid {
public:
    TextGrid(int width, int height);

    void Resize(int width, int height);
    void Clear();
    void Clear(Color bg);
    void Put(int x, int y, char32_t ch, Color fg, Color bg, uint8_t flags = 0);
    void PutString(int x, int y, const std::string& str, Color fg, Color bg, uint8_t flags = 0);
    void PutString(int x, int y, const std::u32string& str, Color fg, Color bg, uint8_t flags = 0);
    void FillRect(const Rect& rect, char32_t ch, Color fg, Color bg, uint8_t flags = 0);
    void FillRect(const Rect& rect, Color bg); // fill with spaces
    void DrawBox(const Rect& rect, Color fg, Color bg); // single-line box drawing

    Cell& At(int x, int y);
    const Cell& At(int x, int y) const;

    int Width() const { return width_; }
    int Height() const { return height_; }

    const std::vector<Cell>& Cells() const { return cells_; }

    // Font selection: subsequent Put/PutString/FillRect/DrawBox calls use this font.
    // Empty name or "default" maps to index 0.
    void SetFont(const std::string& name);
    const std::string& GetFont() const { return fontNames_[currentFont_]; }
    uint16_t GetFontIndex() const { return currentFont_; }
    const std::vector<std::string>& FontNames() const { return fontNames_; }

    void PushFont(const std::string& name);
    void PopFont();

private:
    int width_ = 0;
    int height_ = 0;
    std::vector<Cell> cells_;

    std::vector<std::string> fontNames_ = {{"default"}};
    std::vector<uint16_t> fontStack_;
    uint16_t currentFont_ = 0;

    uint16_t FontIndex(const std::string& name);
};

} // namespace bui
