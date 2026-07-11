#include "basicui/widget/progressbar.h"
#include "basicui/render/text_grid.h"
#include "basicui/style/theme.h"
#include <algorithm>
#include <sstream>

namespace bui {

void ProgressBar::SetRange(int min, int max) {
    min_ = min;
    max_ = max;
    if (min_ > max_) std::swap(min_, max_);
    SetValue(value_);
}

ProgressBar& ProgressBar::SetValue(int value) {
    value_ = value;
    if (value_ < min_) value_ = min_;
    if (value_ > max_) value_ = max_;
    return *this;
}

ProgressBar& ProgressBar::SetBounds(const Rect& r) {
    Widget::SetBounds(r);
    return *this;
}

void ProgressBar::OnRender(TextGrid& grid) {
    if (!visible_) return;
    int w = bounds_.w;
    int h = bounds_.h;
    if (w <= 0 || h <= 0) return;

    float ratio = (max_ > min_) ? static_cast<float>(value_ - min_) / (max_ - min_) : 0.0f;
    int fill = static_cast<int>(ratio * w);
    if (fill > w) fill = w;

    std::string bar;
    for (int i = 0; i < w; ++i) {
        if (i < fill) bar += '=';
        else bar += '-';
    }

    Color fillColor = Theme::SelectionBg;
    Color emptyColor = Theme::DefaultBg;

    for (int row = 0; row < h; ++row) {
        for (int col = 0; col < w; ++col) {
            Color bg = (col < fill) ? fillColor : emptyColor;
            grid.Put(bounds_.x + col, bounds_.y + row, ' ', fgColor_, bg);
        }
    }

    if (showPercent_ && w >= 5) {
        std::ostringstream oss;
        oss << static_cast<int>(ratio * 100) << "%";
        std::string pct = oss.str();
        int pctX = bounds_.x + (w - static_cast<int>(pct.size())) / 2;
        int pctY = bounds_.y + h / 2;
        if (pctX >= bounds_.x && pctX + static_cast<int>(pct.size()) <= bounds_.x + w) {
            for (size_t i = 0; i < pct.size(); ++i) {
                int col = pctX + static_cast<int>(i) - bounds_.x;
                Color bg = (col < fill) ? fillColor : emptyColor;
                grid.Put(pctX + static_cast<int>(i), pctY, pct[i], fgColor_, bg);
            }
        }
    }
}

Point ProgressBar::GetPreferredSize() const {
    return {20, 1};
}

} // namespace bui
