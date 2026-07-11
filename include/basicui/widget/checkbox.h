#pragma once

#include "basicui/widget/widget.h"

namespace bui {

class CheckBox : public Widget {
public:
    explicit CheckBox(const std::string& label = "") : label_(label) {
        focusable_ = true;
    }

    void SetLabel(const std::string& l) { label_ = l; }
    const std::string& GetLabel() const { return label_; }
    CheckBox& Label(const std::string& l) { label_ = l; return *this; }

    void SetChecked(bool c) { checked_ = c; }
    bool IsChecked() const { return checked_; }
    CheckBox& Checked(bool c) { checked_ = c; return *this; }

    void OnEvent(const Event& ev) override;
    void OnRender(TextGrid& grid) override;
    Point GetPreferredSize() const override;

    void OnToggle(std::function<void(bool)> cb) { onToggle_ = std::move(cb); }
    CheckBox& Toggled(std::function<void(bool)> cb) { onToggle_ = std::move(cb); return *this; }

private:
    std::string label_;
    bool checked_ = false;
    std::function<void(bool)> onToggle_;
};

} // namespace bui
