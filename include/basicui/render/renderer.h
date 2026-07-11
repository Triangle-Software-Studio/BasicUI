#pragma once

#include <string>
#include <vector>
#include <memory>
#include "basicui/basicui.h"

namespace bui {

class TextGrid;
class GlyphAtlas;
class Window;

class Renderer {
public:
    Renderer();
    ~Renderer();

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    bool Initialize(Window* window);
    void Shutdown();

    // The default atlas is used for grid metrics and for cells with the default font.
    void SetGlyphAtlas(std::shared_ptr<GlyphAtlas> atlas);

    // Register a named font atlas for per-widget font switching.
    void SetFontAtlas(const std::string& name, std::shared_ptr<GlyphAtlas> atlas);

    // Render the current TextGrid contents to screen
    void Render(const TextGrid& grid, int gridCols, int gridRows);

    // Draw window decorations (title bar, borders, scroll bar) in character cells
    // These are drawn *after* the grid content, overlay style.
    void DrawTitleBar(const std::string& title, int cols, bool active, int& outTitleBarRows);
    void DrawBorder(int cols, int rows, int titleBarRows, bool active);
    void DrawScrollBar(int cols, int rows, int scrollPos, int contentHeight, int titleBarRows);

    void SwapBuffers();

    void SetViewport(int pixelWidth, int pixelHeight);

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace bui
