#pragma once

#include "basicui/widget/widget.h"
#include <functional>
#include <string>

namespace bui {

class RadioButton : public Widget {
public:
    RadioButton(const std::string& label = "");
    ~RadioButton();

    void SetLabel(const std::string& label) { label_ = label; }
    const std::string& GetLabel() const { return label_; }
    RadioButton& Label(const std::string& label) { label_ = label; return *this; }

    RadioButton& SetChecked(bool checked);
    bool IsChecked() const { return checked_; }
    RadioButton& Checked(bool checked) { return SetChecked(checked); }

    RadioButton& SetGroup(const std::string& group);
    const std::string& GetGroup() const { return group_; }
    RadioButton& Group(const std::string& group) { return SetGroup(group); }

    void OnToggle(std::function<void(bool)> cb) { onToggle_ = cb; }
    RadioButton& Toggled(std::function<void(bool)> cb) { onToggle_ = cb; return *this; }

    void OnEvent(const Event& ev) override;
    void OnRender(TextGrid& grid) override;
    Point GetPreferredSize() const override;

private:
    std::string label_;
    std::string group_;
    bool checked_ = false;
    std::function<void(bool)> onToggle_;
};

} // namespace bui
