#pragma once

#include "basicui/layout/layout.h"

namespace bui {

class GridLayout : public Layout {
public:
    GridLayout(int cols, int rows) : cols_(cols), rows_(rows) {}

    void Arrange(const Rect& bounds, const std::vector<std::shared_ptr<Widget>>& children) override;
    Point GetPreferredSize(const std::vector<std::shared_ptr<Widget>>& children) override;

    void SetGap(int colGap, int rowGap) { colGap_ = colGap; rowGap_ = rowGap; }

private:
    int cols_ = 1;
    int rows_ = 1;
    int colGap_ = 0;
    int rowGap_ = 0;
};

} // namespace bui
