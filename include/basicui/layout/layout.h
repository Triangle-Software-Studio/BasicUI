#pragma once

#include <memory>
#include <vector>
#include "basicui/basicui.h"

namespace bui {

class Widget;

class Layout {
public:
    virtual ~Layout() = default;

    // Arrange children within the given rectangle (in cell coordinates)
    virtual void Arrange(const Rect& bounds, const std::vector<std::shared_ptr<Widget>>& children) = 0;

    // Compute preferred size for the given children
    virtual Point GetPreferredSize(const std::vector<std::shared_ptr<Widget>>& children) = 0;
};

} // namespace bui
