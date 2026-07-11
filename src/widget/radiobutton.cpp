#include "basicui/widget/radiobutton.h"
#include "basicui/render/text_grid.h"
#include "basicui/style/theme.h"

namespace bui {

void RadioButton::OnEvent(const Event& ev) {
    if (!enabled_) return;

    if (ev.type == EventType::MouseDown) {
        if (Contains(ev.mouse.x, ev.mouse.y)) {
            if (!checked_) {
                checked_ = true;
                if (onToggle_) onToggle_(true);
            }
        }
    } else if (ev.type == EventType::KeyPress) {
        if (focused_ && (ev.key.key == KeyCode::Space || ev.key.key == KeyCode::Enter)) {
            if (!checked_) {
                checked_ = true;
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
        int labelW = std::min(static_cast<int>(label_.size()), w - 4);
        grid.PutString(bounds_.x + 4, bounds_.y, label_.substr(0, labelW), fg, bg);
    }
}

Point RadioButton::GetPreferredSize() const {
    return {static_cast<int>(4 + label_.size()), 1};
}

} // namespace bui
