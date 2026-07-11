#pragma once

#include "basicui/widget/widget.h"

namespace bui {

// Root container that fills the entire grid
class Screen : public Widget {
public:
    Screen() {}

    void OnRender(TextGrid& grid) override;
    Point GetPreferredSize() const override;

    // Focus management
    std::shared_ptr<Widget> FocusedWidget() const { return focusedWidget_.lock(); }
    bool MoveFocus(int delta); // +1 or -1 tab order
    bool SetFocusTo(std::shared_ptr<Widget> w);

    void OnEvent(const Event& ev) override;

private:
    void CollectFocusable(std::vector<std::shared_ptr<Widget>>& out, std::shared_ptr<Widget> root);
    void CollectVisible(std::vector<std::shared_ptr<Widget>>& out, std::shared_ptr<Widget> root);
    std::weak_ptr<Widget> focusedWidget_;
};

} // namespace bui
