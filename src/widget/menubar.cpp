#include "basicui/widget/menubar.h"
#include "basicui/render/text_grid.h"
#include "basicui/style/theme.h"

namespace bui {

void MenuBar::AddMenu(const std::string& title, const std::vector<Item>& items) {
    menus_.push_back({title, items});
}

void MenuBar::GetDropDownBounds(int& outX, int& outY, int& outW, int& outH) const {
    outX = bounds_.x;
    outY = bounds_.y;
    outW = 0;
    outH = 0;
    if (activeMenu_ < 0 || activeMenu_ >= static_cast<int>(menus_.size())) return;

    const auto& menu = menus_[activeMenu_];
    int maxLen = 0;
    for (const auto& item : menu.items) {
        if (static_cast<int>(item.label.size()) > maxLen) maxLen = static_cast<int>(item.label.size());
    }
    outW = maxLen + 2;
    outH = static_cast<int>(menu.items.size()) + 2;

    outX = bounds_.x;
    for (int i = 0; i < activeMenu_; ++i) {
        outX += static_cast<int>(menus_[i].title.size()) + 3;
    }
    outY = bounds_.y + 1;

    if (outX + outW > bounds_.x + bounds_.w) outW = bounds_.x + bounds_.w - outX;
    if (outY + outH > bounds_.y + bounds_.h) outH = bounds_.y + bounds_.h - outY;
    if (outW < 0) outW = 0;
    if (outH < 0) outH = 0;
}

bool MenuBar::OnHitTest(int x, int y) const {
    if (Contains(x, y)) return true;
    if (activeMenu_ >= 0) {
        int dropX, dropY, dropW, dropH;
        GetDropDownBounds(dropX, dropY, dropW, dropH);
        if (dropW > 0 && dropH > 0) {
            if (x >= dropX && x < dropX + dropW && y >= dropY && y < dropY + dropH) {
                return true;
            }
        }
    }
    return false;
}

void MenuBar::OnRender(TextGrid& grid) {
    if (!visible_) return;
    int x = bounds_.x;
    int y = bounds_.y;

    grid.FillRect({bounds_.x, bounds_.y, bounds_.w, 1}, ' ', Theme::DefaultFg, Theme::MenuBarBg);

    for (size_t i = 0; i < menus_.size(); ++i) {
        std::string label = "[" + menus_[i].title + "]";
        if (x + static_cast<int>(label.size()) > bounds_.x + bounds_.w) break;
        Color bg = (static_cast<int>(i) == hoveredMenu_) ? Theme::MenuHighlightBg : Theme::MenuBarBg;
        grid.PutString(x, y, label, Theme::DefaultFg, bg);
        x += static_cast<int>(label.size()) + 1;
    }
}

void MenuBar::OnRenderOverlay(TextGrid& grid) {
    if (!visible_) return;
    if (activeMenu_ < 0 || activeMenu_ >= static_cast<int>(menus_.size())) return;

    int dropX, dropY, dropW, dropH;
    GetDropDownBounds(dropX, dropY, dropW, dropH);

    if (dropW > 0 && dropH > 0) {
        grid.DrawBox({dropX, dropY, dropW, dropH}, Theme::DefaultFg, Theme::MenuBarBg);
        for (size_t i = 0; i < menus_[activeMenu_].items.size(); ++i) {
            int itemY = dropY + 1 + static_cast<int>(i);
            if (itemY >= dropY + dropH - 1) break;
            Color bg = (static_cast<int>(i) == hoveredItem_) ? Theme::MenuHighlightBg : Theme::MenuBarBg;
            std::string label = " " + menus_[activeMenu_].items[i].label;
            if (static_cast<int>(label.size()) > dropW - 2) {
                label = label.substr(0, dropW - 2);
            }
            grid.PutString(dropX + 1, itemY, label, Theme::DefaultFg, bg);
        }
    }
}

Point MenuBar::GetPreferredSize() const {
    int totalWidth = 0;
    for (size_t i = 0; i < menus_.size(); ++i) {
        totalWidth += static_cast<int>(menus_[i].title.size()) + 3;
    }
    if (!menus_.empty()) totalWidth -= 1;
    return {totalWidth, 1};
}

void MenuBar::OnEvent(const Event& ev) {
    if (!enabled_) return;
    switch (ev.type) {
    case EventType::MouseDown: {
        int x = bounds_.x;
        for (size_t i = 0; i < menus_.size(); ++i) {
            int titleWidth = static_cast<int>(menus_[i].title.size()) + 2;
            if (ev.mouse.x >= x && ev.mouse.x < x + titleWidth && ev.mouse.y == bounds_.y) {
                activeMenu_ = (activeMenu_ == static_cast<int>(i)) ? -1 : static_cast<int>(i);
                hoveredMenu_ = static_cast<int>(i);
                hoveredItem_ = -1;
                break;
            }
            x += titleWidth + 1;
        }
        break;
    }
    case EventType::MouseMove: {
        if (activeMenu_ >= 0) {
            int dropX, dropY, dropW, dropH;
            GetDropDownBounds(dropX, dropY, dropW, dropH);
            if (dropW > 0 && dropH > 0 &&
                ev.mouse.x >= dropX && ev.mouse.x < dropX + dropW &&
                ev.mouse.y >= dropY && ev.mouse.y < dropY + dropH) {
                hoveredItem_ = ev.mouse.y - dropY - 1;
                if (hoveredItem_ < 0 || hoveredItem_ >= static_cast<int>(menus_[activeMenu_].items.size())) {
                    hoveredItem_ = -1;
                }
            } else {
                hoveredItem_ = -1;
            }
        } else {
            int x = bounds_.x;
            hoveredMenu_ = -1;
            for (size_t i = 0; i < menus_.size(); ++i) {
                int titleWidth = static_cast<int>(menus_[i].title.size()) + 2;
                if (ev.mouse.x >= x && ev.mouse.x < x + titleWidth && ev.mouse.y == bounds_.y) {
                    hoveredMenu_ = static_cast<int>(i);
                    break;
                }
                x += titleWidth + 1;
            }
        }
        break;
    }
    case EventType::MouseUp: {
        if (activeMenu_ >= 0 && hoveredItem_ >= 0 && hoveredItem_ < static_cast<int>(menus_[activeMenu_].items.size())) {
            if (menus_[activeMenu_].items[hoveredItem_].action) {
                menus_[activeMenu_].items[hoveredItem_].action();
            }
            CloseMenu();
        }
        break;
    }
    case EventType::KeyPress:
        if (ev.key.key == KeyCode::Escape) {
            CloseMenu();
        }
        break;
    default:
        break;
    }
}

} // namespace bui
