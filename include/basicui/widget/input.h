#pragma once

#include "basicui/widget/widget.h"

namespace bui {

class Input : public Widget {
public:
    Input() {
        focusable_ = true;
    }

    void SetText(const std::string& t) { text_ = t; cursorPos_ = static_cast<int>(t.size()); }
    const std::string& GetText() const { return text_; }
    Input& Text(const std::string& t) { SetText(t); return *this; }

    Input& SetPlaceholder(const std::string& p) { placeholder_ = p; return *this; }
    const std::string& GetPlaceholder() const { return placeholder_; }
    Input& Placeholder(const std::string& p) { return SetPlaceholder(p); }

    Input& SetMaxLength(int len) { maxLength_ = len; return *this; }
    Input& MaxLength(int len) { return SetMaxLength(len); }

    void OnEvent(const Event& ev) override;
    void OnRender(TextGrid& grid) override;
    Point GetPreferredSize() const override;

    Input& SetBounds(const Rect& r) override;

    void OnSubmit(std::function<void(const std::string&)> cb) { onSubmit_ = std::move(cb); }
    Input& Submitted(std::function<void(const std::string&)> cb) { onSubmit_ = std::move(cb); return *this; }
    void OnChange(std::function<void(const std::string&)> cb) { onChange_ = std::move(cb); }
    Input& Changed(std::function<void(const std::string&)> cb) { onChange_ = std::move(cb); return *this; }

private:
    void InsertText(const std::string& str);
    void InsertChar(char32_t ch);
    void DeleteBackward();
    void DeleteForward();
    void MoveCursor(int delta);

    std::string text_;
    std::string placeholder_;
    int cursorPos_ = 0;
    int maxLength_ = 0; // 0 = unlimited
    int scrollOffset_ = 0;
    std::function<void(const std::string&)> onSubmit_;
    std::function<void(const std::string&)> onChange_;
};

} // namespace bui
