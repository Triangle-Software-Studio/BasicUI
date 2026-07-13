#include "basicui/widget/checkbox.h"
#include "basicui/render/text_grid.h"
#include "basicui/style/theme.h"

namespace bui {

void CheckBox::OnRender(TextGrid& grid) {
    if (!visible_) return;
    std::string display = (checked_ ? "[x] " : "[ ] ") + label_;
    if (static_cast<int>(TextGrid::Utf8CharCount(display)) > bounds_.w) {
        display = TextGrid::Utf8Substr(display, 0, bounds_.w);
    }
    grid.PutString(bounds_.x, bounds_.y, display, fgColor_, bgColor_);
}

Point CheckBox::GetPreferredSize() const {
    return {static_cast<int>(TextGrid::Utf8CharCount(label_)) + 4, 1};
}

void CheckBox::OnEvent(const Event& ev) {
    if (!enabled_) return;
    switch (ev.type) {
    case EventType::MouseDown:
        if (bounds_.Contains(ev.mouse.x, ev.mouse.y)) {
            checked_ = !checked_;
            if (onToggle_) onToggle_(checked_);
        }
        break;
    case EventType::KeyPress:
        if (focused_ && ev.key.key == KeyCode::Space) {
            checked_ = !checked_;
            if (onToggle_) onToggle_(checked_);
        }
        break;
    default:
        break;
    }
}

} // namespace bui
