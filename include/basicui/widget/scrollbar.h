#pragma once

#include "basicui/widget/widget.h"

namespace bui {

class ScrollBar : public Widget {
public:
    enum Orientation { Vertical, Horizontal };

    explicit ScrollBar(Orientation orient = Vertical) : orientation_(orient) {}

    void SetOrientation(Orientation o) { orientation_ = o; }
    Orientation GetOrientation() const { return orientation_; }

    ScrollBar& SetRange(int contentSize, int viewportSize);
    ScrollBar& Range(int contentSize, int viewportSize) { return SetRange(contentSize, viewportSize); }
    ScrollBar& SetPosition(int pos); // scroll position (0..contentSize-viewportSize)
    ScrollBar& Position(int pos) { return SetPosition(pos); }
    int GetPosition() const { return position_; }
    int GetContentSize() const { return contentSize_; }
    int GetViewportSize() const { return viewportSize_; }

    void OnEvent(const Event& ev) override;
    void OnRender(TextGrid& grid) override;
    Point GetPreferredSize() const override;

    void OnScroll(std::function<void(int)> cb) { onScroll_ = std::move(cb); }
    ScrollBar& Scrolled(std::function<void(int)> cb) { onScroll_ = std::move(cb); return *this; }

private:
    Orientation orientation_ = Vertical;
    int contentSize_ = 0;
    int viewportSize_ = 0;
    int position_ = 0;
    std::function<void(int)> onScroll_;
};

} // namespace bui
