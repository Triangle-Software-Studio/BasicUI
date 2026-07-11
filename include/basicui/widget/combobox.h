#pragma once

#include "basicui/widget/widget.h"
#include <functional>
#include <string>
#include <vector>

namespace bui {

class ComboBox : public Widget {
public:
    ComboBox() {}

    ComboBox& AddItem(const std::string& item);
    ComboBox& Item(const std::string& item) { return AddItem(item); }
    void ClearItems();
    void SetSelectedIndex(int idx);
    ComboBox& SelectedIndex(int idx) { SetSelectedIndex(idx); return *this; }
    int GetSelectedIndex() const { return selectedIndex_; }
    const std::string& GetSelectedItem() const;

    void OnSelect(std::function<void(int, const std::string&)> cb) { onSelect_ = cb; }
    ComboBox& Selected(std::function<void(int, const std::string&)> cb) { onSelect_ = cb; return *this; }

    void OnEvent(const Event& ev) override;
    void OnRender(TextGrid& grid) override;
    void OnRenderOverlay(TextGrid& grid) override;
    bool OnHitTest(int x, int y) const override;
    Point GetPreferredSize() const override;

private:
    std::vector<std::string> items_;
    int selectedIndex_ = -1;
    bool expanded_ = false;
    std::function<void(int, const std::string&)> onSelect_;
};

} // namespace bui
