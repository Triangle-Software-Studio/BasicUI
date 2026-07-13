#include "basicui/layout/flex_layout.h"
#include "basicui/widget/widget.h"
#include <algorithm>

namespace bui {

void FlexLayout::Arrange(const Rect& bounds, const std::vector<std::shared_ptr<Widget>>& children) {
    int innerW = bounds.w - padLeft_ - padRight_;
    int innerH = bounds.h - padTop_ - padBottom_;
    if (innerW < 0) innerW = 0;
    if (innerH < 0) innerH = 0;

    int count = static_cast<int>(children.size());
    int totalGap = (count > 1) ? (count - 1) * gap_ : 0;

    int totalPref = 0;
    for (const auto& c : children) {
        Point p = c->GetPreferredSize();
        int sz = (direction_ == Vertical) ? p.y : p.x;
        if (sz == 0) sz = 1;
        totalPref += sz;
    }

    int available = ((direction_ == Vertical) ? innerH : innerW) - totalGap;
    if (available < count) available = count;

    bool bounded = (totalPref > available);

    int x = bounds.x + padLeft_;
    int y = bounds.y + padTop_;

    int childIndex = 0;
    for (const auto& c : children) {
        Point p = c->GetPreferredSize();
        int sz = (direction_ == Vertical) ? p.y : p.x;
        if (sz == 0) sz = 1;

        if (bounded) {
            int base = available / count;
            int rem = available % count;
            sz = base;
            if (sz < 1) sz = 1;
            // Distribute remainder
            if (base >= 1 && childIndex < rem) {
                sz = base + 1;
            }
        }
        ++childIndex;

        Rect cb;
        if (direction_ == Vertical) {
            cb = {x, y, innerW, sz};
            y += sz + gap_;
        } else {
            cb = {x, y, sz, innerH};
            x += sz + gap_;
        }
        c->SetBounds(cb);
    }
}

Point FlexLayout::GetPreferredSize(const std::vector<std::shared_ptr<Widget>>& children) {
    int totalW = 0, totalH = 0;
    int count = static_cast<int>(children.size());
    int totalGap = (count > 1) ? (count - 1) * gap_ : 0;

    for (const auto& c : children) {
        Point p = c->GetPreferredSize();
        if (p.x == 0) p.x = 1;
        if (p.y == 0) p.y = 1;
        if (direction_ == Vertical) {
            totalH += p.y;
            totalW = std::max(totalW, p.x);
        } else {
            totalW += p.x;
            totalH = std::max(totalH, p.y);
        }
    }

    if (direction_ == Vertical) {
        return {totalW + padLeft_ + padRight_, totalH + totalGap + padTop_ + padBottom_};
    } else {
        return {totalW + totalGap + padLeft_ + padRight_, totalH + padTop_ + padBottom_};
    }
}

} // namespace bui
