#include "basicui/widget/tooltip.h"
#include "basicui/render/text_grid.h"
#include "basicui/style/theme.h"
#include <algorithm>

namespace bui {

Tooltip::Tooltip(const std::string& text) : text_(text) {
    visible_ = false;
    zIndex_ = 100; // Tooltips render on top of everything
}

void Tooltip::SetText(const std::string& text) {
    text_ = text;
}

Point Tooltip::GetPreferredSize() const {
    int w = std::min(maxWidth_, static_cast<int>(text_.size()));
    if (w <= 0) w = 1;
    int lines = (static_cast<int>(text_.size()) + w - 1) / w;
    return {w + 2, lines + 2};
}

void Tooltip::ShowAt(int x, int y, int maxWidth) {
    maxWidth_ = maxWidth;
    Point ps = GetPreferredSize();
    SetBounds({x, y, ps.x, ps.y});
    visible_ = true;
    shown_ = true;
}

void Tooltip::AttachTo(std::shared_ptr<Widget> target, const std::string& text) {
    text_ = text;
    if (!target) return;
    Rect b = target->Bounds();
    ShowAt(b.x, b.y + b.h, b.w);
}

static std::vector<std::string> WrapText(const std::string& text, int maxWidth) {
    std::vector<std::string> lines;
    std::string current;
    int len = 0;
    for (const char* p = text.c_str(); *p; ) {
        char32_t ch = 0;
        size_t step = 1;
        unsigned char c = static_cast<unsigned char>(*p);
        if (c < 0x80) { ch = c; step = 1; }
        else if ((c & 0xE0) == 0xC0) { ch = ((c & 0x1F) << 6) | (p[1] & 0x3F); step = 2; }
        else if ((c & 0xF0) == 0xE0) { ch = ((c & 0x0F) << 12) | ((p[1] & 0x3F) << 6) | (p[2] & 0x3F); step = 3; }
        else if ((c & 0xF8) == 0xF0) { ch = ((c & 0x07) << 18) | ((p[1] & 0x3F) << 12) | ((p[2] & 0x3F) << 6) | (p[3] & 0x3F); step = 4; }
        else { ch = c; step = 1; }

        if (ch == U'\n') {
            lines.push_back(current);
            current.clear();
            len = 0;
        } else {
            if (len >= maxWidth && len > 0) {
                lines.push_back(current);
                current.clear();
                len = 0;
            }
            current += std::string(p, step);
            len++;
        }
        p += step;
    }
    if (!current.empty() || lines.empty()) {
        lines.push_back(current);
    }
    return lines;
}

void Tooltip::OnRender(TextGrid& grid) {
    // Tooltip renders in overlay pass so it is always on top
}

void Tooltip::OnRenderOverlay(TextGrid& grid) {
    if (!visible_ || !shown_ || text_.empty()) return;

    auto lines = WrapText(text_, bounds_.w - 2);
    grid.DrawBox(bounds_, Theme::BorderActive, Theme::ButtonBg);
    for (size_t i = 0; i < lines.size() && i < static_cast<size_t>(bounds_.h - 2); ++i) {
        grid.PutString(bounds_.x + 1, bounds_.y + 1 + static_cast<int>(i), lines[i], fgColor_, Theme::ButtonBg);
    }
}

// ---------------------------------------------------------------------------
// TooltipManager
// ---------------------------------------------------------------------------

TooltipManager& TooltipManager::Instance() {
    static TooltipManager inst;
    return inst;
}

void TooltipManager::Register(std::shared_ptr<Widget> widget, const std::string& text) {
    if (!widget) return;
    tips_[widget.get()] = text;
}

void TooltipManager::Unregister(std::shared_ptr<Widget> widget) {
    if (!widget) return;
    tips_.erase(widget.get());
}

void TooltipManager::OnHover(std::shared_ptr<Widget> widget, int x, int y) {
    if (!widget) {
        Hide();
        return;
    }
    auto it = tips_.find(widget.get());
    if (it == tips_.end()) {
        Hide();
        return;
    }
    if (auto t = activeTarget_.lock(); t == widget) {
        return;
    }
    activeTarget_ = widget;
    if (!activeTip_) activeTip_ = std::make_shared<Tooltip>();
    activeTip_->SetText(it->second);
    activeTip_->ShowAt(x + 1, y + 1, 40);
}

void TooltipManager::Hide() {
    activeTarget_.reset();
    if (activeTip_) activeTip_->SetVisible(false);
}

void TooltipManager::RenderOverlay(TextGrid& grid) {
    if (activeTip_) activeTip_->RenderOverlay(grid);
}

} // namespace bui
