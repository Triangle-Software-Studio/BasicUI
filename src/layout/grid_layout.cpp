#include "basicui/layout/grid_layout.h"
#include "basicui/widget/widget.h"
#include <algorithm>

namespace bui {

void GridLayout::Arrange(const Rect& bounds, const std::vector<std::shared_ptr<Widget>>& children) {
    int gapsW = (cols_ > 1) ? (cols_ - 1) * colGap_ : 0;
    int gapsH = (rows_ > 1) ? (rows_ - 1) * rowGap_ : 0;
    int cellW = (cols_ > 0) ? (bounds.w - gapsW) / cols_ : 0;
    int cellH = (rows_ > 0) ? (bounds.h - gapsH) / rows_ : 0;

    int idx = 0;
    for (const auto& c : children) {
        if (idx >= cols_ * rows_) break;
        int col = idx % cols_;
        int row = idx / cols_;
        int x = bounds.x + col * (cellW + colGap_);
        int y = bounds.y + row * (cellH + rowGap_);
        c->SetBounds({x, y, cellW, cellH});
        ++idx;
    }
}

Point GridLayout::GetPreferredSize(const std::vector<std::shared_ptr<Widget>>& children) {
    int maxW = 0, maxH = 0;
    for (const auto& c : children) {
        Point p = c->GetPreferredSize();
        if (p.x > maxW) maxW = p.x;
        if (p.y > maxH) maxH = p.y;
    }
    if (maxW == 0) maxW = 1;
    if (maxH == 0) maxH = 1;

    int totalW = cols_ * maxW + ((cols_ > 1) ? (cols_ - 1) * colGap_ : 0);
    int totalH = rows_ * maxH + ((rows_ > 1) ? (rows_ - 1) * rowGap_ : 0);
    return {totalW, totalH};
}

} // namespace bui
