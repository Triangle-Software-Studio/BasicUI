#pragma once

#include "basicui/widget/widget.h"
#include "basicui/widget/panel.h"
#include <functional>
#include <string>
#include <vector>

namespace bui {

// Tab page item: label + content panel
struct TabPage {
    std::string label;
    std::shared_ptr<Panel> content;
};

class TabControl : public Widget {
public:
    TabControl();

    // Builder-style API
    TabControl& WithTab(const std::string& label, std::shared_ptr<Panel> content);
    TabControl& OnChange(std::function<void(int, const std::string&)> cb);

    // Add a tab programmatically
    void AddTab(const std::string& label, std::shared_ptr<Panel> content);

    void SetSelectedIndex(int idx);
    TabControl& SelectedIndex(int idx) { SetSelectedIndex(idx); return *this; }
    int GetSelectedIndex() const { return selectedIndex_; }
    const std::string& GetSelectedLabel() const;

    void OnEvent(const Event& ev) override;
    void OnRender(TextGrid& grid) override;
    void OnRenderOverlay(TextGrid& grid) override;
    bool OnHitTest(int x, int y) const override;
    Point GetPreferredSize() const override;

    Widget& SetBounds(const Rect& r) override;

private:
    Rect TabBarRect() const;
    Rect ContentRect() const;
    int TabWidth() const;

    std::vector<TabPage> tabs_;
    int selectedIndex_ = 0;
    std::function<void(int, const std::string&)> onChange_;
    Color borderColor_ = MakeColor(128, 128, 128, 255);
};

} // namespace bui
