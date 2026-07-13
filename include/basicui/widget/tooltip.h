#pragma once

#include "basicui/widget/widget.h"
#include <string>
#include <vector>
#include <utility>
#include <cstdint>

namespace bui {

// A simple tooltip widget that can be attached to any widget.
// TooltipManager handles the global shared tooltip popup.
class Tooltip : public Widget {
public:
    explicit Tooltip(const std::string& text = "");

    void SetText(const std::string& text);
    const std::string& GetText() const { return text_; }

    // Position the tooltip near a target widget or point
    void ShowAt(int x, int y, int maxWidth);
    void AttachTo(std::shared_ptr<Widget> target, const std::string& text);

    void OnRender(TextGrid& grid) override;
    void OnRenderOverlay(TextGrid& grid) override;
    bool OnHitTest(int x, int y) const override { return false; }
    Point GetPreferredSize() const override;

    void SetDelayMs(int ms) { delayMs_ = ms; }
    int GetDelayMs() const { return delayMs_; }

private:
    std::string text_;
    int delayMs_ = 600;
    int maxWidth_ = 40;
    bool shown_ = false;

    friend class TooltipManager;
};

// Optional helper: shows tooltip for any widget with a tooltip text set on it.
class TooltipManager {
public:
    static TooltipManager& Instance();

    void Register(std::shared_ptr<Widget> widget, const std::string& text);
    void Unregister(std::shared_ptr<Widget> widget);

    void OnHover(std::shared_ptr<Widget> widget, int x, int y);
    void Hide();

    void RenderOverlay(TextGrid& grid);

private:
    TooltipManager() = default;

    std::vector<std::pair<std::weak_ptr<Widget>, std::string>> tips_;
    std::shared_ptr<Tooltip> activeTip_;
    std::weak_ptr<Widget> activeTarget_;
    uint32_t hoverStartTime_ = 0;
    int hoverX_ = 0;
    int hoverY_ = 0;
};

} // namespace bui
