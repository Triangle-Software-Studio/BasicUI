#pragma once

#include "basicui/widget/widget.h"

namespace bui {

class ProgressBar : public Widget {
public:
    ProgressBar() {}

    void SetRange(int min, int max);
    ProgressBar& Range(int min, int max) { SetRange(min, max); return *this; }
    ProgressBar& SetValue(int value);
    ProgressBar& Value(int v) { return SetValue(v); }
    int GetValue() const { return value_; }
    int GetMin() const { return min_; }
    int GetMax() const { return max_; }

    ProgressBar& SetShowPercent(bool show) { showPercent_ = show; return *this; }
    ProgressBar& ShowPercent(bool show) { return SetShowPercent(show); }

    void OnRender(TextGrid& grid) override;
    Point GetPreferredSize() const override;

    ProgressBar& SetBounds(const Rect& r) override;

private:
    int min_ = 0;
    int max_ = 100;
    int value_ = 0;
    bool showPercent_ = true;
};

} // namespace bui
