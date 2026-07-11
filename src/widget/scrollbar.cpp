#include "basicui/widget/scrollbar.h"
#include "basicui/render/text_grid.h"
#include "basicui/style/theme.h"
#include <algorithm>

namespace bui {

ScrollBar& ScrollBar::SetRange(int contentSize, int viewportSize) {
    contentSize_ = contentSize;
    viewportSize_ = viewportSize;
    int maxPos = contentSize_ - viewportSize_;
    if (maxPos < 0) maxPos = 0;
    if (position_ > maxPos) position_ = maxPos;
    if (position_ < 0) position_ = 0;
    return *this;
}

ScrollBar& ScrollBar::SetPosition(int pos) {
    position_ = pos;
    int maxPos = contentSize_ - viewportSize_;
    if (maxPos < 0) maxPos = 0;
    if (position_ > maxPos) position_ = maxPos;
    if (position_ < 0) position_ = 0;
    return *this;
}

void ScrollBar::OnRender(TextGrid& grid) {
    if (!visible_) return;
    if (orientation_ == Vertical) {
        for (int y = bounds_.y; y < bounds_.y + bounds_.h; ++y) {
            grid.Put(bounds_.x, y, U'\u2502', Theme::ScrollBarBg, bgColor_);
        }
        if (contentSize_ > 0 && viewportSize_ > 0 && viewportSize_ < contentSize_) {
            int thumbSize = std::max(1, bounds_.h * viewportSize_ / contentSize_);
            int maxPos = contentSize_ - viewportSize_;
            int thumbPos = (maxPos > 0) ? (position_ * (bounds_.h - thumbSize) / maxPos) : 0;
            for (int i = 0; i < thumbSize; ++i) {
                int y = bounds_.y + thumbPos + i;
                if (y >= bounds_.y && y < bounds_.y + bounds_.h) {
                    grid.Put(bounds_.x, y, U'\u2588', Theme::ScrollBarThumb, bgColor_);
                }
            }
        }
    } else {
        for (int x = bounds_.x; x < bounds_.x + bounds_.w; ++x) {
            grid.Put(x, bounds_.y, U'\u2500', Theme::ScrollBarBg, bgColor_);
        }
        if (contentSize_ > 0 && viewportSize_ > 0 && viewportSize_ < contentSize_) {
            int thumbSize = std::max(1, bounds_.w * viewportSize_ / contentSize_);
            int maxPos = contentSize_ - viewportSize_;
            int thumbPos = (maxPos > 0) ? (position_ * (bounds_.w - thumbSize) / maxPos) : 0;
            for (int i = 0; i < thumbSize; ++i) {
                int x = bounds_.x + thumbPos + i;
                if (x >= bounds_.x && x < bounds_.x + bounds_.w) {
                    grid.Put(x, bounds_.y, U'\u2588', Theme::ScrollBarThumb, bgColor_);
                }
            }
        }
    }
}

Point ScrollBar::GetPreferredSize() const {
    if (orientation_ == Vertical) {
        return {1, std::max(1, bounds_.h)};
    } else {
        return {std::max(1, bounds_.w), 1};
    }
}

void ScrollBar::OnEvent(const Event& ev) {
    if (!enabled_) return;
    switch (ev.type) {
    case EventType::MouseDown:
        if (bounds_.Contains(ev.mouse.x, ev.mouse.y) && contentSize_ > 0 && viewportSize_ < contentSize_) {
            if (orientation_ == Vertical) {
                int thumbSize = std::max(1, bounds_.h * viewportSize_ / contentSize_);
                int relY = ev.mouse.y - bounds_.y - thumbSize / 2;
                int maxPos = contentSize_ - viewportSize_;
                int newPos = (bounds_.h - thumbSize > 0) ? (relY * maxPos / (bounds_.h - thumbSize)) : 0;
                SetPosition(newPos);
            } else {
                int thumbSize = std::max(1, bounds_.w * viewportSize_ / contentSize_);
                int relX = ev.mouse.x - bounds_.x - thumbSize / 2;
                int maxPos = contentSize_ - viewportSize_;
                int newPos = (bounds_.w - thumbSize > 0) ? (relX * maxPos / (bounds_.w - thumbSize)) : 0;
                SetPosition(newPos);
            }
            if (onScroll_) onScroll_(position_);
        }
        break;
    case EventType::MouseWheel:
        SetPosition(position_ - ev.mouse.delta);
        if (onScroll_) onScroll_(position_);
        break;
    default:
        break;
    }
}

} // namespace bui
