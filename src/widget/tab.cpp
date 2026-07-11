#include "basicui/widget/tab.h"
#include "basicui/render/text_grid.h"
#include "basicui/style/theme.h"
#include <algorithm>

namespace bui {

TabControl::TabControl() {
    focusable_ = true;
    zIndex_ = 1; // Tabs should draw above siblings when needed
}

void TabControl::AddTab(const std::string& label, std::shared_ptr<Panel> content) {
    if (!content) return;
    tabs_.push_back({label, content});
    content->SetVisible(static_cast<int>(tabs_.size()) - 1 == selectedIndex_);
    content->SetBounds(ContentRect());
    AddChild(content);
}

Widget& TabControl::SetBounds(const Rect& r) {
    bounds_ = r;
    for (auto& t : tabs_) {
        t.content->SetBounds(ContentRect());
    }
    return *this;
}

TabControl& TabControl::WithTab(const std::string& label, std::shared_ptr<Panel> content) {
    AddTab(label, content);
    return *this;
}

TabControl& TabControl::OnChange(std::function<void(int, const std::string&)> cb) {
    onChange_ = std::move(cb);
    return *this;
}

void TabControl::SetSelectedIndex(int idx) {
    if (idx < 0 || idx >= static_cast<int>(tabs_.size())) return;
    selectedIndex_ = idx;
    for (size_t i = 0; i < tabs_.size(); ++i) {
        tabs_[i].content->SetVisible(static_cast<int>(i) == selectedIndex_);
    }
    if (onChange_) onChange_(selectedIndex_, tabs_[selectedIndex_].label);
}

const std::string& TabControl::GetSelectedLabel() const {
    static const std::string empty;
    if (selectedIndex_ < 0 || selectedIndex_ >= static_cast<int>(tabs_.size())) return empty;
    return tabs_[selectedIndex_].label;
}

Rect TabControl::TabBarRect() const {
    return {bounds_.x, bounds_.y, bounds_.w, 1};
}

Rect TabControl::ContentRect() const {
    return {bounds_.x, bounds_.y + 1, bounds_.w, bounds_.h - 1};
}

int TabControl::TabWidth() const {
    if (tabs_.empty()) return 0;
    int w = bounds_.w / static_cast<int>(tabs_.size());
    if (w < 6) w = 6;
    return w;
}

void TabControl::OnRender(TextGrid& grid) {
    if (!visible_ || tabs_.empty()) return;

    Rect tabBar = TabBarRect();
    int tw = TabWidth();

    // Draw tab bar background
    grid.FillRect(tabBar, Theme::ButtonBg);

    for (size_t i = 0; i < tabs_.size(); ++i) {
        int tx = tabBar.x + static_cast<int>(i) * tw;
        int cw = (i == tabs_.size() - 1) ? tabBar.Right() - tx : tw;
        Color bg = (static_cast<int>(i) == selectedIndex_) ? Theme::SelectionBg : Theme::ButtonBg;
        Color fg = (static_cast<int>(i) == selectedIndex_) ? Theme::SelectionFg : fgColor_;

        grid.FillRect({tx, tabBar.y, cw, 1}, ' ', fg, bg);

        std::string label = " " + tabs_[i].label + " ";
        if (static_cast<int>(label.size()) > cw) {
            label = label.substr(0, cw);
        }
        grid.PutString(tx, tabBar.y, label, fg, bg);
    }

    // Draw content area border/background
    Rect content = ContentRect();
    grid.DrawBox(content, borderColor_, bgColor_);

    // Render selected tab content (children visibility controls the rest)
    for (const auto& child : children_) {
        if (child->IsVisible()) child->Render(grid);
    }
}

void TabControl::OnRenderOverlay(TextGrid& grid) {
    // Tabs themselves do not need overlay, but their children might
    for (const auto& child : children_) {
        if (child->IsVisible()) child->RenderOverlay(grid);
    }
}

bool TabControl::OnHitTest(int x, int y) const {
    if (bounds_.Contains(x, y)) return true;
    // Allow children (content panels) to receive events if visible
    if (selectedIndex_ >= 0 && selectedIndex_ < static_cast<int>(tabs_.size())) {
        return tabs_[selectedIndex_].content->HitTest(x, y);
    }
    return false;
}

void TabControl::OnEvent(const Event& ev) {
    if (!enabled_) return;

    if (ev.type == EventType::MouseDown) {
        Rect tabBar = TabBarRect();
        if (tabBar.Contains(ev.mouse.x, ev.mouse.y)) {
            int tw = TabWidth();
            int idx = (ev.mouse.x - tabBar.x) / tw;
            SetSelectedIndex(idx);
        } else {
            // Forward to visible content panel
            if (selectedIndex_ >= 0 && selectedIndex_ < static_cast<int>(tabs_.size())) {
                tabs_[selectedIndex_].content->OnEvent(ev);
            }
        }
    } else if (ev.type == EventType::KeyPress) {
        if (focused_) {
            if (ev.key.key == KeyCode::Left || ev.key.key == KeyCode::Up) {
                if (selectedIndex_ > 0) SetSelectedIndex(selectedIndex_ - 1);
            } else if (ev.key.key == KeyCode::Right || ev.key.key == KeyCode::Down) {
                if (selectedIndex_ + 1 < static_cast<int>(tabs_.size())) {
                    SetSelectedIndex(selectedIndex_ + 1);
                }
            }
        }
    } else {
        // Other events go to visible content
        if (selectedIndex_ >= 0 && selectedIndex_ < static_cast<int>(tabs_.size())) {
            tabs_[selectedIndex_].content->OnEvent(ev);
        }
    }
}

Point TabControl::GetPreferredSize() const {
    int maxW = 12;
    int maxH = 5;
    for (const auto& t : tabs_) {
        Point p = t.content->GetPreferredSize();
        if (p.x > maxW) maxW = p.x;
        if (p.y > maxH) maxH = p.y;
    }
    return {maxW, maxH + 1};
}

} // namespace bui
