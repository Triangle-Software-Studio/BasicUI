#pragma once

#include <memory>
#include <vector>
#include <string>
#include "basicui/basicui.h"
#include "basicui/platform/event.h"

namespace bui {

class TextGrid;
class Layout;
class GlyphAtlas;

// Base class for all UI widgets
class Widget : public std::enable_shared_from_this<Widget> {
public:
    Widget() = default;
    virtual ~Widget() = default;

    virtual void OnEvent(const Event& ev) {}

    // Public render entry point. Derived classes override OnRender.
    void Render(TextGrid& grid);
    virtual void OnRender(TextGrid& grid) = 0;

    virtual Rect Bounds() const { return bounds_; }
    virtual Widget& SetBounds(const Rect& r) { bounds_ = r; return *this; }

    // Fluent bounds setter
    Widget& At(int x, int y) { bounds_.x = x; bounds_.y = y; return *this; }
    Widget& Size(int w, int h) { bounds_.w = w; bounds_.h = h; return *this; }
    Widget& Place(int x, int y, int w, int h) { bounds_ = {x, y, w, h}; return *this; }

    virtual bool IsVisible() const { return visible_; }
    virtual void SetVisible(bool v) { visible_ = v; }
    Widget& Visible(bool v) { visible_ = v; return *this; }

    virtual bool IsEnabled() const { return enabled_; }
    virtual void SetEnabled(bool e) { enabled_ = e; }
    Widget& Enabled(bool e) { enabled_ = e; return *this; }

    virtual bool IsFocusable() const { return focusable_; }
    virtual void SetFocusable(bool f) { focusable_ = f; }

    bool Contains(int x, int y) const { return bounds_.Contains(x, y); }

    // Hit testing: includes overlay area for pop-up widgets
    bool HitTest(int x, int y) const { return OnHitTest(x, y); }
    virtual bool OnHitTest(int x, int y) const { return Contains(x, y); }

    // Whether child widgets are clipped to this widget's bounds
    virtual bool ClipChildren() const { return true; }

    // Z-Index for render order (higher = rendered later / on top)
    void SetZIndex(int z) { zIndex_ = z; }
    int GetZIndex() const { return zIndex_; }
    Widget& ZIndex(int z) { zIndex_ = z; return *this; }

    // Overlay render pass: drawn after all normal widgets
    void RenderOverlay(TextGrid& grid) { OnRenderOverlay(grid); }
    virtual void OnRenderOverlay(TextGrid& grid) {}

    // Layout helpers
    virtual Point GetPreferredSize() const { return {1,1}; }
    virtual Point GetMinimumSize() const { return {1,1}; }
    virtual Point GetMaximumSize() const { return {9999,9999}; }

    // Parent / child tree
    std::shared_ptr<Widget> Parent() const { return parent_.lock(); }
    void SetParent(std::shared_ptr<Widget> p) { parent_ = p; }
    const std::vector<std::shared_ptr<Widget>>& Children() const { return children_; }
    void AddChild(std::shared_ptr<Widget> child);
    Widget& WithChild(std::shared_ptr<Widget> child) { AddChild(child); return *this; }
    void RemoveChild(std::shared_ptr<Widget> child);
    void ClearChildren();

    void SetLayout(std::shared_ptr<Layout> layout) { layout_ = layout; }
    std::shared_ptr<Layout> GetLayout() const { return layout_; }

    void SetBackgroundColor(Color c) { bgColor_ = c; }
    Color GetBackgroundColor() const { return bgColor_; }
    Widget& Background(Color c) { bgColor_ = c; return *this; }

    void SetForegroundColor(Color c) { fgColor_ = c; }
    Color GetForegroundColor() const { return fgColor_; }
    Widget& Foreground(Color c) { fgColor_ = c; return *this; }

    // Font selection. Empty string means "use renderer default / global font".
    void SetFont(const std::string& fontName) { fontName_ = fontName; }
    const std::string& GetFont() const { return fontName_; }
    Widget& Font(const std::string& fontName) { fontName_ = fontName; return *this; }

    // Focus
    void SetFocused(bool f) { focused_ = f; }
    bool IsFocused() const { return focused_; }

protected:
    Rect bounds_;
    bool visible_ = true;
    bool enabled_ = true;
    bool focusable_ = false;
    bool focused_ = false;
    int zIndex_ = 0;
    Color bgColor_ = MakeColor(12,12,12,255);
    Color fgColor_ = MakeColor(204,204,204,255);
    std::string fontName_;
    std::weak_ptr<Widget> parent_;
    std::vector<std::shared_ptr<Widget>> children_;
    std::shared_ptr<Layout> layout_;
};

} // namespace bui
