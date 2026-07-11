#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <algorithm>
#include <utility>

namespace bui {

using Color = uint32_t; // 0xAABBGGRR or 0xAARRGGBB? Let's use 0xAARRGGBB consistent with most APIs.

inline constexpr Color MakeColor(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) {
    return (static_cast<Color>(a) << 24) | (static_cast<Color>(r) << 16) | (static_cast<Color>(g) << 8) | static_cast<Color>(b);
}

inline constexpr uint8_t ColorR(Color c) { return static_cast<uint8_t>((c >> 16) & 0xFF); }
inline constexpr uint8_t ColorG(Color c) { return static_cast<uint8_t>((c >> 8) & 0xFF); }
inline constexpr uint8_t ColorB(Color c) { return static_cast<uint8_t>(c & 0xFF); }
inline constexpr uint8_t ColorA(Color c) { return static_cast<uint8_t>((c >> 24) & 0xFF); }

struct Point {
    int x = 0;
    int y = 0;
};

struct Rect {
    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;

    bool Contains(int px, int py) const {
        return px >= x && px < x + w && py >= y && py < y + h;
    }
    bool Contains(const Point& p) const { return Contains(p.x, p.y); }

    int Right() const { return x + w; }
    int Bottom() const { return y + h; }
};

} // namespace bui
