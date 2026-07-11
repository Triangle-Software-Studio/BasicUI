#pragma once

#include <string>
#include <vector>
#include <functional>
#include "basicui/widget/widget.h"

namespace bui {

class MenuBar : public Widget {
public:
    struct Item {
        std::string label;
        std::function<void()> action;
    };
    struct Menu {
        std::string title;
        std::vector<Item> items;
    };

    MenuBar() {}

    void AddMenu(const std::string& title, const std::vector<Item>& items);
    MenuBar& WithMenu(const std::string& title, const std::vector<Item>& items) {
        AddMenu(title, items); return *this;
    }

    void OnEvent(const Event& ev) override;
    void OnRender(TextGrid& grid) override;
    void OnRenderOverlay(TextGrid& grid) override;
    bool OnHitTest(int x, int y) const override;
    Point GetPreferredSize() const override;

    void SetActiveMenu(int index) { activeMenu_ = index; }
    int GetActiveMenu() const { return activeMenu_; }

    void CloseMenu() { activeMenu_ = -1; }

private:
    void GetDropDownBounds(int& outX, int& outY, int& outW, int& outH) const;

    std::vector<Menu> menus_;
    int activeMenu_ = -1;
    int hoveredMenu_ = -1;
    int hoveredItem_ = -1;
};

} // namespace bui
