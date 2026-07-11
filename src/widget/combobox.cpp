#include "basicui/widget/combobox.h"
#include "basicui/render/text_grid.h"
#include "basicui/style/theme.h"

namespace bui {

ComboBox& ComboBox::AddItem(const std::string& item) {
    items_.push_back(item);
    if (selectedIndex_ < 0 && !items_.empty()) {
        selectedIndex_ = 0;
    }
    return *this;
}

void ComboBox::ClearItems() {
    items_.clear();
    selectedIndex_ = -1;
}

void ComboBox::SetSelectedIndex(int idx) {
    if (idx >= -1 && idx < static_cast<int>(items_.size())) {
        selectedIndex_ = idx;
    }
}

const std::string& ComboBox::GetSelectedItem() const {
    static const std::string empty;
    if (selectedIndex_ >= 0 && selectedIndex_ < static_cast<int>(items_.size())) {
        return items_[selectedIndex_];
    }
    return empty;
}

bool ComboBox::OnHitTest(int x, int y) const {
    if (Contains(x, y)) return true;
    if (expanded_) {
        int maxItems = std::min(static_cast<int>(items_.size()), 5);
        int dropY0 = bounds_.y + 1;
        int dropY1 = dropY0 + maxItems;
        if (x >= bounds_.x && x < bounds_.x + bounds_.w && y >= dropY0 && y < dropY1) {
            return true;
        }
    }
    return false;
}

void ComboBox::OnEvent(const Event& ev) {
    if (!enabled_) return;

    if (ev.type == EventType::MouseDown) {
        if (Contains(ev.mouse.x, ev.mouse.y)) {
            expanded_ = !expanded_;
        } else if (expanded_ && HitTest(ev.mouse.x, ev.mouse.y)) {
            int itemIndex = ev.mouse.y - bounds_.y - 1;
            if (itemIndex >= 0 && itemIndex < static_cast<int>(items_.size())) {
                selectedIndex_ = itemIndex;
                if (onSelect_) onSelect_(selectedIndex_, items_[selectedIndex_]);
            }
            expanded_ = false;
        } else if (expanded_) {
            expanded_ = false;
        }
    } else if (ev.type == EventType::KeyPress) {
        if (focused_) {
            if (ev.key.key == KeyCode::Space || ev.key.key == KeyCode::Enter) {
                expanded_ = !expanded_;
            } else if (expanded_ && ev.key.key == KeyCode::Up) {
                if (selectedIndex_ > 0) {
                    selectedIndex_--;
                    if (onSelect_) onSelect_(selectedIndex_, items_[selectedIndex_]);
                }
            } else if (expanded_ && ev.key.key == KeyCode::Down) {
                if (selectedIndex_ + 1 < static_cast<int>(items_.size())) {
                    selectedIndex_++;
                    if (onSelect_) onSelect_(selectedIndex_, items_[selectedIndex_]);
                }
            }
        }
    }
}

void ComboBox::OnRender(TextGrid& grid) {
    if (!visible_) return;
    int w = bounds_.w;
    if (w <= 0) return;

    Color bg = focused_ ? Theme::SelectionBg : Theme::DefaultBg;
    Color fg = fgColor_;

    std::string display = (selectedIndex_ >= 0 && selectedIndex_ < static_cast<int>(items_.size()))
                              ? items_[selectedIndex_]
                              : "";
    std::string text = "[" + display + "]";
    if (static_cast<int>(text.size()) > w) {
        text = text.substr(0, w - 1) + "]";
    }
    int padding = w - static_cast<int>(text.size());
    if (padding < 0) padding = 0;
    text += std::string(padding, ' ');
    text.back() = expanded_ ? '^' : 'v';

    grid.PutString(bounds_.x, bounds_.y, text, fg, bg);
}

void ComboBox::OnRenderOverlay(TextGrid& grid) {
    if (!visible_ || !expanded_) return;
    int w = bounds_.w;
    if (w <= 0) return;

    Color fg = fgColor_;
    int maxItems = std::min(static_cast<int>(items_.size()), 5);
    for (int i = 0; i < maxItems; ++i) {
        int rowY = bounds_.y + 1 + i;
        Color itemBg = (i == selectedIndex_) ? Theme::SelectionBg : Theme::DefaultBg;
        std::string itemText = items_[i];
        if (static_cast<int>(itemText.size()) > w) {
            itemText = itemText.substr(0, w);
        }
        int itemPadding = w - static_cast<int>(itemText.size());
        if (itemPadding < 0) itemPadding = 0;
        itemText += std::string(itemPadding, ' ');
        grid.PutString(bounds_.x, rowY, itemText, fg, itemBg);
    }
}

Point ComboBox::GetPreferredSize() const {
    int maxLen = 10;
    for (const auto& item : items_) {
        if (static_cast<int>(item.size()) > maxLen) maxLen = static_cast<int>(item.size());
    }
    return {maxLen + 4, 1};
}

} // namespace bui
