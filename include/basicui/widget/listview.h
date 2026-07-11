#pragma once

#include <string>
#include <vector>
#include <functional>
#include "basicui/widget/widget.h"

namespace bui {

class ListView : public Widget {
public:
    ListView() {}

    ListView& AddItem(const std::string& item);
    ListView& Item(const std::string& item) { return AddItem(item); }
    void RemoveItem(size_t index);
    void Clear();

    void SetSelectedIndex(int idx) { selectedIndex_ = idx; }
    ListView& SelectedIndex(int idx) { selectedIndex_ = idx; return *this; }
    int GetSelectedIndex() const { return selectedIndex_; }
    const std::string& GetItem(size_t index) const;
    size_t ItemCount() const { return items_.size(); }

    void OnEvent(const Event& ev) override;
    void OnRender(TextGrid& grid) override;
    Point GetPreferredSize() const override;

    void OnSelect(std::function<void(int, const std::string&)> cb) { onSelect_ = std::move(cb); }
    ListView& Selected(std::function<void(int, const std::string&)> cb) { onSelect_ = std::move(cb); return *this; }

    void SetScrollOffset(int off) { scrollOffset_ = off; }
    int GetScrollOffset() const { return scrollOffset_; }

private:
    std::vector<std::string> items_;
    int selectedIndex_ = -1;
    int scrollOffset_ = 0;
    std::function<void(int, const std::string&)> onSelect_;
};

} // namespace bui
