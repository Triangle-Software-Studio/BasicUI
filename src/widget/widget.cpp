#include "basicui/widget/widget.h"
#include "basicui/render/text_grid.h"
#include <algorithm>

namespace bui {

void Widget::Render(TextGrid& grid) {
    std::string prev = grid.GetFont();
    if (!fontName_.empty()) {
        grid.SetFont(fontName_);
    }
    OnRender(grid);
    grid.SetFont(prev);
}

void Widget::AddChild(std::shared_ptr<Widget> child) {
    if (child) {
        child->SetParent(shared_from_this());
        children_.push_back(child);
    }
}

void Widget::RemoveChild(std::shared_ptr<Widget> child) {
    auto it = std::find(children_.begin(), children_.end(), child);
    if (it != children_.end()) {
        (*it)->SetParent(nullptr);
        children_.erase(it);
    }
}

void Widget::ClearChildren() {
    for (auto& c : children_) {
        c->SetParent(nullptr);
    }
    children_.clear();
}

} // namespace bui
