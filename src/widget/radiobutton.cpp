#include "basicui/widget/radiobutton.h"
#include "basicui/render/text_grid.h"
#include "basicui/style/theme.h"
#include <unordered_map>
#include <vector>
#include <algorithm>

namespace {
std::unordered_map<std::string, std::vector<bui::RadioButton*>>& Groups() {
    static std::unordered_map<std::string, std::vector<bui::RadioButton*>> g;
    return g;
}
}

namespace bui {

RadioButton::RadioButton(const std::string& label) : label_(label) {
    focusable_ = true;
}

RadioButton::~RadioButton() {
    if (!group_.empty()) {
        auto& vec = Groups()[group_];
        vec.erase(std::remove(vec.begin(), vec.end(), this), vec.end());
        if (vec.empty()) Groups().erase(group_);
    }
}

RadioButton& RadioButton::SetGroup(const std::string& group) {
    if (!group_.empty()) {
        auto& vec = Groups()[group_];
        vec.erase(std::remove(vec.begin(), vec.end(), this), vec.end());
        if (vec.empty()) Groups().erase(group_);
    }
    group_ = group;
    if (!group_.empty()) {
        Groups()[group_].push_back(this);
    }
    return *this;
}

RadioButton& RadioButton::SetChecked(bool checked) {
    checked_ = checked;
    if (checked_ && !group_.empty()) {
        auto it = Groups().find(group_);
        if (it != Groups().end()) {
            for (auto* other : it->second) {
                if (other != this && other->checked_) {
                    other->checked_ = false;
                    if (other->onToggle_) other->onToggle_(false);
                }
            }
        }
    }
    return *this;
}

void RadioButton::OnEvent(const Event& ev) {
    if (!enabled_) return;

    if (ev.type == EventType::MouseDown) {
        if (Contains(ev.mouse.x, ev.mouse.y)) {
            if (!checked_) {
                SetChecked(true);
                if (onToggle_) onToggle_(true);
            }
        }
    } else if (ev.type == EventType::KeyPress) {
        if (focused_ && (ev.key.key == KeyCode::Space || ev.key.key == KeyCode::Enter)) {
            if (!checked_) {
                SetChecked(true);
                if (onToggle_) onToggle_(true);
            }
        }
    }
}

void RadioButton::OnRender(TextGrid& grid) {
    if (!visible_) return;
    int w = bounds_.w;
    int h = bounds_.h;
    if (w <= 0 || h <= 0) return;

    Color bg = focused_ ? Theme::SelectionBg : bgColor_;
    Color fg = fgColor_;

    std::string indicator = checked_ ? "(o)" : "( )";
    grid.PutString(bounds_.x, bounds_.y, indicator, fg, bg);

    if (!label_.empty() && w > 4) {
        int labelW = std::min(static_cast<int>(TextGrid::Utf8CharCount(label_)), w - 4);
        grid.PutString(bounds_.x + 4, bounds_.y, TextGrid::Utf8Substr(label_, 0, labelW), fg, bg);
    }
}

Point RadioButton::GetPreferredSize() const {
    return {static_cast<int>(4 + TextGrid::Utf8CharCount(label_)), 1};
}

} // namespace bui
