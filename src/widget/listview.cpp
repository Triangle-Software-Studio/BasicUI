#include "basicui/widget/listview.h"
#include "basicui/render/text_grid.h"
#include "basicui/style/theme.h"

namespace bui {

ListView& ListView::AddItem(const std::string& item) {
    items_.push_back(item);
    return *this;
}

void ListView::RemoveItem(size_t index) {
    if (index < items_.size()) {
        items_.erase(items_.begin() + index);
        if (selectedIndex_ >= static_cast<int>(items_.size())) {
            selectedIndex_ = static_cast<int>(items_.size()) - 1;
        }
        if (selectedIndex_ < 0 && !items_.empty()) selectedIndex_ = 0;
    }
}

void ListView::Clear() {
    items_.clear();
    selectedIndex_ = -1;
    scrollOffset_ = 0;
}

const std::string& ListView::GetItem(size_t index) const {
    static const std::string empty;
    if (index < items_.size()) return items_[index];
    return empty;
}

void ListView::OnRender(TextGrid& grid) {
    if (!visible_) return;
    int x = bounds_.x;
    int y = bounds_.y;
    int w = bounds_.w;
    int h = bounds_.h;

    for (int row = 0; row < h; ++row) {
        int itemIdx = scrollOffset_ + row;
        if (itemIdx < 0 || itemIdx >= static_cast<int>(items_.size())) {
            grid.FillRect({x, y + row, w, 1}, ' ', fgColor_, bgColor_);
            continue;
        }

        const std::string& item = items_[itemIdx];
        Color bg = (itemIdx == selectedIndex_) ? Theme::SelectionBg : bgColor_;
        Color fg = (itemIdx == selectedIndex_) ? Theme::SelectionFg : fgColor_;

        std::string display = item;
        if (static_cast<int>(TextGrid::Utf8CharCount(display)) > w) {
            display = TextGrid::Utf8Substr(display, 0, w);
        }

        grid.FillRect({x, y + row, w, 1}, ' ', fg, bg);
        grid.PutString(x, y + row, display, fg, bg);
    }
}

Point ListView::GetPreferredSize() const {
    int maxLen = 0;
    for (const auto& item : items_) {
        if (static_cast<int>(TextGrid::Utf8CharCount(item)) > maxLen) maxLen = static_cast<int>(TextGrid::Utf8CharCount(item));
    }
    int h = static_cast<int>(items_.size());
    if (bounds_.h > 0 && h > bounds_.h) h = bounds_.h;
    return {maxLen, h};
}

void ListView::OnEvent(const Event& ev) {
    if (!enabled_) return;
    switch (ev.type) {
    case EventType::MouseDown: {
        if (bounds_.Contains(ev.mouse.x, ev.mouse.y)) {
            int idx = scrollOffset_ + (ev.mouse.y - bounds_.y);
            if (idx >= 0 && idx < static_cast<int>(items_.size())) {
                selectedIndex_ = idx;
                if (onSelect_) onSelect_(selectedIndex_, items_[selectedIndex_]);
            }
        }
        break;
    }
    case EventType::KeyPress: {
        if (ev.key.key == KeyCode::Up) {
            if (selectedIndex_ > 0) {
                --selectedIndex_;
                if (selectedIndex_ < scrollOffset_) scrollOffset_ = selectedIndex_;
                if (onSelect_) onSelect_(selectedIndex_, items_[selectedIndex_]);
            }
        } else if (ev.key.key == KeyCode::Down) {
            if (selectedIndex_ < static_cast<int>(items_.size()) - 1) {
                ++selectedIndex_;
                if (selectedIndex_ >= scrollOffset_ + bounds_.h) {
                    scrollOffset_ = selectedIndex_ - bounds_.h + 1;
                }
                if (onSelect_) onSelect_(selectedIndex_, items_[selectedIndex_]);
            }
        }
        break;
    }
    case EventType::MouseWheel: {
        scrollOffset_ -= ev.mouse.delta;
        if (scrollOffset_ < 0) scrollOffset_ = 0;
        int maxScroll = static_cast<int>(items_.size()) - bounds_.h;
        if (maxScroll < 0) maxScroll = 0;
        if (scrollOffset_ > maxScroll) scrollOffset_ = maxScroll;
        break;
    }
    default:
        break;
    }
}

} // namespace bui
