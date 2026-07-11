#pragma once

#include "basicui/widget/widget.h"

namespace bui {

class Button : public Widget {
public:
    explicit Button(const std::string& label = "Button") : label_(label) {
        focusable_ = true;
    }

    void SetLabel(const std::string& l) { label_ = l; }
    const std::string& GetLabel() const { return label_; }
    Button& Label(const std::string& l) { label_ = l; return *this; }

    void OnEvent(const Event& ev) override;
    void OnRender(TextGrid& grid) override;
    Point GetPreferredSize() const override;

    void OnClick(std::function<void()> cb) { onClick_ = std::move(cb); }
    Button& Clicked(std::function<void()> cb) { onClick_ = std::move(cb); return *this; }

    bool IsPressed() const { return pressed_; }
    bool IsHovered() const { return hovered_; }

private:
    std::string label_;
    std::function<void()> onClick_;
    bool pressed_ = false;
    bool hovered_ = false;
};

} // namespace bui
