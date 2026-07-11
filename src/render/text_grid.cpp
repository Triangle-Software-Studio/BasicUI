#include "basicui/render/text_grid.h"
#include <algorithm>

namespace bui {

TextGrid::TextGrid(int width, int height) {
    Resize(width, height);
}

void TextGrid::Resize(int width, int height) {
    width_ = width;
    height_ = height;
    cells_.resize(static_cast<size_t>(width_) * height_);
    Clear();
}

void TextGrid::Clear() {
    Clear(MakeColor(12, 12, 12, 255));
}

void TextGrid::Clear(Color bg) {
    Cell c;
    c.codepoint = ' ';
    c.fg = MakeColor(204, 204, 204, 255);
    c.bg = bg;
    c.flags = 0;
    c.font = currentFont_;
    std::fill(cells_.begin(), cells_.end(), c);
}

void TextGrid::Put(int x, int y, char32_t ch, Color fg, Color bg, uint8_t flags) {
    if (x < 0 || x >= width_ || y < 0 || y >= height_) return;
    Cell& c = cells_[static_cast<size_t>(y) * width_ + x];
    c.codepoint = ch;
    c.fg = fg;
    c.bg = bg;
    c.flags = flags;
    c.font = currentFont_;
}

static size_t Utf8Decode(const char* str, char32_t& out) {
    unsigned char c = static_cast<unsigned char>(str[0]);
    if (c < 0x80) {
        out = c;
        return 1;
    } else if ((c & 0xE0) == 0xC0) {
        out = ((c & 0x1F) << 6) | (static_cast<unsigned char>(str[1]) & 0x3F);
        return 2;
    } else if ((c & 0xF0) == 0xE0) {
        out = ((c & 0x0F) << 12) | ((static_cast<unsigned char>(str[1]) & 0x3F) << 6) | (static_cast<unsigned char>(str[2]) & 0x3F);
        return 3;
    } else if ((c & 0xF8) == 0xF0) {
        out = ((c & 0x07) << 18) | ((static_cast<unsigned char>(str[1]) & 0x3F) << 12) | ((static_cast<unsigned char>(str[2]) & 0x3F) << 6) | (static_cast<unsigned char>(str[3]) & 0x3F);
        return 4;
    }
    out = c;
    return 1;
}

void TextGrid::PutString(int x, int y, const std::string& str, Color fg, Color bg, uint8_t flags) {
    int cx = x;
    const char* p = str.c_str();
    while (*p) {
        char32_t ch = 0;
        size_t len = Utf8Decode(p, ch);
        if (cx < width_ && y >= 0 && y < height_) {
            Put(cx, y, ch, fg, bg, flags);
        }
        cx++;
        p += len;
    }
}

void TextGrid::PutString(int x, int y, const std::u32string& str, Color fg, Color bg, uint8_t flags) {
    int cx = x;
    for (char32_t ch : str) {
        if (cx < width_ && y >= 0 && y < height_) {
            Put(cx, y, ch, fg, bg, flags);
        }
        cx++;
    }
}

void TextGrid::FillRect(const Rect& rect, char32_t ch, Color fg, Color bg, uint8_t flags) {
    int x0 = std::max(rect.x, 0);
    int y0 = std::max(rect.y, 0);
    int x1 = std::min(rect.Right(), width_);
    int y1 = std::min(rect.Bottom(), height_);
    for (int y = y0; y < y1; ++y) {
        for (int x = x0; x < x1; ++x) {
            Cell& c = cells_[static_cast<size_t>(y) * width_ + x];
            c.codepoint = ch;
            c.fg = fg;
            c.bg = bg;
            c.flags = flags;
            c.font = currentFont_;
        }
    }
}

void TextGrid::FillRect(const Rect& rect, Color bg) {
    int x0 = std::max(rect.x, 0);
    int y0 = std::max(rect.y, 0);
    int x1 = std::min(rect.Right(), width_);
    int y1 = std::min(rect.Bottom(), height_);
    for (int y = y0; y < y1; ++y) {
        for (int x = x0; x < x1; ++x) {
            Cell& c = cells_[static_cast<size_t>(y) * width_ + x];
            c.codepoint = ' ';
            c.fg = MakeColor(204, 204, 204, 255);
            c.bg = bg;
            c.flags = 0;
            c.font = currentFont_;
        }
    }
}

void TextGrid::DrawBox(const Rect& rect, Color fg, Color bg) {
    if (rect.w <= 0 || rect.h <= 0) return;

    int x0 = rect.x;
    int y0 = rect.y;
    int x1 = rect.x + rect.w - 1;
    int y1 = rect.y + rect.h - 1;

    // Corners
    Put(x0, y0, U'\u250C', fg, bg);
    Put(x1, y0, U'\u2510', fg, bg);
    Put(x0, y1, U'\u2514', fg, bg);
    Put(x1, y1, U'\u2518', fg, bg);

    // Horizontal lines
    for (int x = x0 + 1; x < x1; ++x) {
        Put(x, y0, U'\u2500', fg, bg);
        Put(x, y1, U'\u2500', fg, bg);
    }

    // Vertical lines
    for (int y = y0 + 1; y < y1; ++y) {
        Put(x0, y, U'\u2502', fg, bg);
        Put(x1, y, U'\u2502', fg, bg);
    }
}

Cell& TextGrid::At(int x, int y) {
    static Cell dummy;
    if (x < 0 || x >= width_ || y < 0 || y >= height_) {
        return dummy;
    }
    return cells_[static_cast<size_t>(y) * width_ + x];
}

const Cell& TextGrid::At(int x, int y) const {
    static Cell dummy;
    if (x < 0 || x >= width_ || y < 0 || y >= height_) {
        return dummy;
    }
    return cells_[static_cast<size_t>(y) * width_ + x];
}

uint16_t TextGrid::FontIndex(const std::string& name) {
    if (name.empty() || name == "default") return 0;
    for (size_t i = 0; i < fontNames_.size(); ++i) {
        if (fontNames_[i] == name) return static_cast<uint16_t>(i);
    }
    fontNames_.push_back(name);
    return static_cast<uint16_t>(fontNames_.size() - 1);
}

void TextGrid::SetFont(const std::string& name) {
    currentFont_ = FontIndex(name);
}

void TextGrid::PushFont(const std::string& name) {
    fontStack_.push_back(currentFont_);
    currentFont_ = FontIndex(name);
}

void TextGrid::PopFont() {
    if (!fontStack_.empty()) {
        currentFont_ = fontStack_.back();
        fontStack_.pop_back();
    } else {
        currentFont_ = 0;
    }
}

} // namespace bui
