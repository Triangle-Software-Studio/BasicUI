#pragma once

#include "basicui/widget/widget.h"
#include <unordered_map>

namespace bui {

class Panel : public Widget {
public:
    explicit Panel(const std::string& title = "") : title_(title) {}

    void SetTitle(const std::string& t) { title_ = t; }
    const std::string& GetTitle() const { return title_; }
    Panel& Title(const std::string& t) { title_ = t; return *this; }

    void OnRender(TextGrid& grid) override;
    Point GetPreferredSize() const override;

    Widget& SetBounds(const Rect& r) override;

    void SetBorderColor(Color c) { borderColor_ = c; }
    Color GetBorderColor() const { return borderColor_; }
    Panel& Border(Color c) { borderColor_ = c; return *this; }

    // Allow children to draw outside panel bounds (e.g. combo dropdowns)
    bool ClipChildren() const override { return false; }

    // Add a child with bounds relative to this panel's top-left corner.
    // The child's absolute bounds are automatically updated when the panel moves or resizes.
    void AddChildRelative(std::shared_ptr<Widget> child, const Rect& relativeBounds);
    Panel& WithRelative(std::shared_ptr<Widget> child, const Rect& relativeBounds) {
        AddChildRelative(child, relativeBounds); return *this;
    }

private:
    void UpdateChildPositions();

    std::string title_;
    Color borderColor_ = MakeColor(128,128,128,255);
    std::unordered_map<Widget*, Rect> relativeBounds_;
};

} // namespace bui
