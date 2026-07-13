#include "basicui/widget/label.h"
#include "basicui/render/text_grid.h"
#include "basicui/style/theme.h"

namespace bui {

void Label::OnRender(TextGrid& grid) {
    if (!visible_) return;
    int maxLen = bounds_.w;
    if (maxLen <= 0) return;
    std::string display = text_;
    if (static_cast<int>(TextGrid::Utf8CharCount(display)) > maxLen) {
        display = TextGrid::Utf8Substr(display, 0, maxLen);
    }
    grid.PutString(bounds_.x, bounds_.y, display, fgColor_, bgColor_);
}

Point Label::GetPreferredSize() const {
    return {static_cast<int>(TextGrid::Utf8CharCount(text_)), 1};
}

} // namespace bui
