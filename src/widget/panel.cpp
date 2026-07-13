#include "basicui/widget/panel.h"
#include "basicui/render/text_grid.h"
#include "basicui/style/theme.h"

namespace bui {

Widget& Panel::SetBounds(const Rect& r) {
    bounds_ = r;
    UpdateChildPositions();
    return *this;
}

void Panel::AddChildRelative(std::shared_ptr<Widget> child, const Rect& relativeBounds) {
    if (!child) return;
    child->SetParent(shared_from_this());
    children_.push_back(child);
    relativeBounds_[child.get()] = relativeBounds;
    UpdateChildPositions();
}

void Panel::UpdateChildPositions() {
    // Clean up entries for children that have been removed
    for (auto it = relativeBounds_.begin(); it != relativeBounds_.end(); ) {
        bool found = false;
        for (const auto& c : children_) {
            if (c.get() == it->first) { found = true; break; }
        }
        if (!found) it = relativeBounds_.erase(it);
        else ++it;
    }

    for (const auto& child : children_) {
        auto it = relativeBounds_.find(child.get());
        if (it != relativeBounds_.end()) {
            const Rect& rel = it->second;
            child->SetBounds({bounds_.x + rel.x, bounds_.y + rel.y, rel.w, rel.h});
        }
    }
}

void Panel::OnRender(TextGrid& grid) {
    if (!visible_) return;
    grid.DrawBox(bounds_, borderColor_, bgColor_);
    if (!title_.empty() && bounds_.w > 2) {
        int titleLen = static_cast<int>(TextGrid::Utf8CharCount(title_));
        int titleX = bounds_.x + (bounds_.w - titleLen) / 2;
        if (titleX < bounds_.x + 1) titleX = bounds_.x + 1;
        int maxLen = bounds_.w - 2 - (titleX - bounds_.x - 1);
        if (maxLen < 0) maxLen = 0;
        std::string display = title_;
        if (static_cast<int>(TextGrid::Utf8CharCount(display)) > maxLen) display = TextGrid::Utf8Substr(display, 0, maxLen);
        grid.PutString(titleX, bounds_.y, display, fgColor_, bgColor_);
    }
}

Point Panel::GetPreferredSize() const {
    if (!children_.empty()) {
        int maxW = 0, maxH = 0;
        for (const auto& c : children_) {
            Point p = c->GetPreferredSize();
            if (p.x > maxW) maxW = p.x;
            if (p.y > maxH) maxH = p.y;
        }
        return {maxW + 2, maxH + 2};
    }
    return {10, 5};
}

} // namespace bui
