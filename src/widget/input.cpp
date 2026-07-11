#include "basicui/widget/input.h"
#include "basicui/render/text_grid.h"
#include "basicui/style/theme.h"

namespace bui {

Input& Input::SetBounds(const Rect& r) {
    Widget::SetBounds(r);
    return *this;
}

void Input::OnRender(TextGrid& grid) {
    if (!visible_) return;
    grid.FillRect(bounds_, ' ', fgColor_, bgColor_);

    int x = bounds_.x;
    int y = bounds_.y;
    int w = bounds_.w;

    if (text_.empty() && !focused_ && !placeholder_.empty()) {
        std::string ph = placeholder_;
        if (static_cast<int>(ph.size()) > w) ph = ph.substr(0, w);
        Color dim = MakeColor(128, 128, 128, 255);
        grid.PutString(x, y, ph, dim, bgColor_);
    } else {
        int textLen = static_cast<int>(text_.size());
        int displayLen = textLen - scrollOffset_;
        if (displayLen > w) displayLen = w;
        if (displayLen < 0) displayLen = 0;
        if (displayLen > 0) {
            grid.PutString(x, y, text_.substr(scrollOffset_, displayLen), fgColor_, bgColor_);
        }
        if (focused_ && w > 0) {
            int cursorX = cursorPos_ - scrollOffset_;
            if (cursorX >= 0 && cursorX < w) {
                char32_t ch = (cursorPos_ < textLen) ? static_cast<char32_t>(text_[cursorPos_]) : U' ';
                grid.Put(x + cursorX, y, ch, bgColor_, fgColor_);
            }
        }
    }
}

Point Input::GetPreferredSize() const {
    if (maxLength_ > 0) return {maxLength_, 1};
    return {20, 1};
}

void Input::OnEvent(const Event& ev) {
    if (!enabled_) return;
    switch (ev.type) {
    case EventType::MouseDown:
        if (bounds_.Contains(ev.mouse.x, ev.mouse.y)) {
            SetFocused(true);
            cursorPos_ = ev.mouse.x - bounds_.x + scrollOffset_;
            if (cursorPos_ < 0) cursorPos_ = 0;
            if (cursorPos_ > static_cast<int>(text_.size())) cursorPos_ = static_cast<int>(text_.size());
        }
        break;
    case EventType::KeyPress: {
        if (!focused_) return;
        auto k = ev.key.key;
        if (IsPrintable(k)) {
            std::string str(1, static_cast<char>(k));
            InsertText(str);
        } else if (k == KeyCode::Backspace) {
            DeleteBackward();
        } else if (k == KeyCode::Delete) {
            DeleteForward();
        } else if (k == KeyCode::Left) {
            MoveCursor(-1);
        } else if (k == KeyCode::Right) {
            MoveCursor(1);
        } else if (k == KeyCode::Home) {
            cursorPos_ = 0;
        } else if (k == KeyCode::End) {
            cursorPos_ = static_cast<int>(text_.size());
        } else if (k == KeyCode::Enter) {
            if (onSubmit_) onSubmit_(text_);
        }
        // Ensure cursor is visible within the viewport
        if (cursorPos_ < scrollOffset_) scrollOffset_ = cursorPos_;
        if (cursorPos_ > scrollOffset_ + bounds_.w) scrollOffset_ = cursorPos_ - bounds_.w + 1;
        if (scrollOffset_ < 0) scrollOffset_ = 0;
        break;
    }
    default:
        break;
    }
}

void Input::InsertText(const std::string& str) {
    if (maxLength_ > 0 && static_cast<int>(text_.size()) >= maxLength_) return;
    text_.insert(cursorPos_, str);
    cursorPos_ += static_cast<int>(str.size());
    if (maxLength_ > 0 && static_cast<int>(text_.size()) > maxLength_) {
        text_.resize(maxLength_);
        if (cursorPos_ > maxLength_) cursorPos_ = maxLength_;
    }
    if (onChange_) onChange_(text_);
}

void Input::DeleteBackward() {
    if (cursorPos_ > 0) {
        text_.erase(cursorPos_ - 1, 1);
        --cursorPos_;
        if (onChange_) onChange_(text_);
    }
}

void Input::DeleteForward() {
    if (cursorPos_ < static_cast<int>(text_.size())) {
        text_.erase(cursorPos_, 1);
        if (onChange_) onChange_(text_);
    }
}

void Input::MoveCursor(int delta) {
    cursorPos_ += delta;
    if (cursorPos_ < 0) cursorPos_ = 0;
    if (cursorPos_ > static_cast<int>(text_.size())) cursorPos_ = static_cast<int>(text_.size());
}

} // namespace bui
