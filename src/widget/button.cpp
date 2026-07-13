#include "basicui/widget/button.h"
#include "basicui/render/text_grid.h"
#include "basicui/style/theme.h"

namespace bui {

void Button::OnRender(TextGrid& grid) {
    if (!visible_) return;
    Color bg = Theme::ButtonBg;
    if (pressed_ && hovered_) bg = Theme::ButtonPressedBg;
    else if (hovered_) bg = Theme::ButtonHoverBg;
    else bg = Theme::ButtonBg;

    grid.FillRect(bounds_, ' ', fgColor_, bg);

    int labelY = bounds_.y + (bounds_.h - 1) / 2;
    if (bounds_.w >= 1) {
        grid.Put(bounds_.x, labelY, '[', fgColor_, bg);
    }
    if (bounds_.w >= 2) {
        grid.Put(bounds_.x + bounds_.w - 1, labelY, ']', fgColor_, bg);
    }

    if (bounds_.w > 4) {
        int maxLabel = bounds_.w - 4;
        std::string display = label_;
        if (static_cast<int>(TextGrid::Utf8CharCount(display)) > maxLabel) {
            display = TextGrid::Utf8Substr(display, 0, maxLabel);
        }
        grid.PutString(bounds_.x + 2, labelY, display, fgColor_, bg);
    }
}

Point Button::GetPreferredSize() const {
    return {static_cast<int>(TextGrid::Utf8CharCount(label_)) + 4, 3};
}

void Button::OnEvent(const Event& ev) {
    if (!enabled_) return;
    switch (ev.type) {
    case EventType::MouseMove:
        hovered_ = bounds_.Contains(ev.mouse.x, ev.mouse.y);
        break;
    case EventType::MouseDown:
        if (bounds_.Contains(ev.mouse.x, ev.mouse.y)) {
            pressed_ = true;
        }
        break;
    case EventType::MouseUp:
        if (pressed_) {
            pressed_ = false;
            if (bounds_.Contains(ev.mouse.x, ev.mouse.y) && onClick_) {
                onClick_();
            }
        }
        break;
    case EventType::KeyPress:
        if (focused_ && (ev.key.key == KeyCode::Enter || ev.key.key == KeyCode::Space)) {
            if (onClick_) onClick_();
        }
        break;
    default:
        break;
    }
}

} // namespace bui
