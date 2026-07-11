#pragma once

#include "basicui/widget/widget.h"

namespace bui {

class Label : public Widget {
public:
    explicit Label(const std::string& text = "") : text_(text) {}

    void SetText(const std::string& t) { text_ = t; }
    const std::string& GetText() const { return text_; }
    Label& Text(const std::string& t) { text_ = t; return *this; }

    void OnRender(TextGrid& grid) override;
    Point GetPreferredSize() const override;

private:
    std::string text_;
};

} // namespace bui
