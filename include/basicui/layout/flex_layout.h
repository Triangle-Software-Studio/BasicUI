#pragma once

#include "basicui/layout/layout.h"

namespace bui {

class FlexLayout : public Layout {
public:
    enum Direction { Horizontal, Vertical };

    explicit FlexLayout(Direction dir = Vertical) : direction_(dir) {}

    void Arrange(const Rect& bounds, const std::vector<std::shared_ptr<Widget>>& children) override;
    Point GetPreferredSize(const std::vector<std::shared_ptr<Widget>>& children) override;

    void SetGap(int gap) { gap_ = gap; }
    void SetPadding(int left, int top, int right, int bottom) {
        padLeft_ = left; padTop_ = top; padRight_ = right; padBottom_ = bottom;
    }

private:
    Direction direction_;
    int gap_ = 0;
    int padLeft_ = 0, padTop_ = 0, padRight_ = 0, padBottom_ = 0;
};

} // namespace bui
