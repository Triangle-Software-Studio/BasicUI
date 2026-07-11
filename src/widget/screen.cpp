#include "basicui/widget/screen.h"
#include "basicui/render/text_grid.h"
#include "basicui/style/theme.h"
#include "basicui/layout/layout.h"
#include "basicui/widget/tooltip.h"
#include <algorithm>

namespace bui {

static std::shared_ptr<Widget> FindTopmostWidget(std::shared_ptr<Widget> root, int x, int y) {
    if (!root || !root->IsVisible()) return nullptr;

    bool inside = root->HitTest(x, y);
    if (!inside && root->ClipChildren()) return nullptr;

    for (int i = static_cast<int>(root->Children().size()) - 1; i >= 0; --i) {
        auto found = FindTopmostWidget(root->Children()[i], x, y);
        if (found) return found;
    }

    return inside ? root : nullptr;
}

void Screen::CollectVisible(std::vector<std::shared_ptr<Widget>>& out, std::shared_ptr<Widget> root) {
    if (!root || !root->IsVisible()) return;
    out.push_back(root);
    for (const auto& child : root->Children()) {
        CollectVisible(out, child);
    }
}

void Screen::OnRender(TextGrid& grid) {
    grid.Clear(Theme::DefaultBg);

    // Collect all visible widgets recursively
    std::vector<std::shared_ptr<Widget>> allWidgets;
    CollectVisible(allWidgets, shared_from_this());

    // Sort by z-index (stable to preserve tree order for equal z)
    std::stable_sort(allWidgets.begin(), allWidgets.end(),
        [](const auto& a, const auto& b) { return a->GetZIndex() < b->GetZIndex(); });

    // Normal render pass (skip self to avoid infinite recursion)
    for (auto& w : allWidgets) {
        if (w.get() == this) continue;
        auto layout = w->GetLayout();
        if (layout) {
            layout->Arrange(w->Bounds(), w->Children());
        }
        w->Render(grid);
    }

    // Overlay render pass (popups, dropdowns, etc.)
    for (auto& w : allWidgets) {
        if (w.get() == this) continue;
        w->RenderOverlay(grid);
    }

    // Global tooltip overlay
    TooltipManager::Instance().RenderOverlay(grid);
}

Point Screen::GetPreferredSize() const {
    return {1, 1};
}

void Screen::OnEvent(const Event& ev) {
    switch (ev.type) {
    case EventType::KeyPress:
    case EventType::KeyRelease: {
        auto fw = focusedWidget_.lock();
        if (fw && fw->IsEnabled()) {
            fw->OnEvent(ev);
        }
        if (ev.type == EventType::KeyPress && ev.key.key == KeyCode::Tab) {
            int delta = (ev.key.modifiers & 1) ? -1 : 1;
            MoveFocus(delta);
        }
        break;
    }
    case EventType::MouseMove:
    case EventType::MouseDown:
    case EventType::MouseUp:
    case EventType::MouseWheel: {
        auto target = FindTopmostWidget(shared_from_this(), ev.mouse.x, ev.mouse.y);
        if (ev.type == EventType::MouseMove) {
            if (target && target.get() != this) {
                TooltipManager::Instance().OnHover(target, ev.mouse.x, ev.mouse.y);
            } else {
                TooltipManager::Instance().Hide();
            }
        }
        if (target && target.get() != this) {
            if (ev.type == EventType::MouseDown) {
                SetFocusTo(target);
            }
            if (target->IsEnabled()) {
                target->OnEvent(ev);
            }
        }
        break;
    }
    case EventType::Resize:
        bounds_ = {0, 0, ev.resize.width, ev.resize.height};
        break;
    default:
        break;
    }
}

bool Screen::MoveFocus(int delta) {
    std::vector<std::shared_ptr<Widget>> focusable;
    CollectFocusable(focusable, shared_from_this());
    if (focusable.empty()) return false;

    int current = -1;
    auto fw = focusedWidget_.lock();
    for (size_t i = 0; i < focusable.size(); ++i) {
        if (focusable[i] == fw) {
            current = static_cast<int>(i);
            break;
        }
    }

    int next = current + delta;
    int count = static_cast<int>(focusable.size());
    if (next < 0) next = count - 1;
    if (next >= count) next = 0;

    return SetFocusTo(focusable[next]);
}

bool Screen::SetFocusTo(std::shared_ptr<Widget> w) {
    if (!w || !w->IsFocusable()) return false;
    if (auto fw = focusedWidget_.lock()) {
        fw->SetFocused(false);
    }
    focusedWidget_ = w;
    w->SetFocused(true);
    return true;
}

void Screen::CollectFocusable(std::vector<std::shared_ptr<Widget>>& out, std::shared_ptr<Widget> root) {
    if (!root) return;
    if (root->IsFocusable() && root->IsVisible() && root->IsEnabled()) {
        out.push_back(root);
    }
    for (const auto& child : root->Children()) {
        CollectFocusable(out, child);
    }
}

} // namespace bui
