#pragma once

#include <string>
#include <memory>
#include <functional>
#include <vector>
#include "basicui/basicui.h"
#include "basicui/platform/event.h"

struct SDL_Window;

namespace bui {

class Window {
public:
    Window(const std::string& title, int cols, int rows, int fontSize = 16);
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    bool IsOpen() const;
    void Close();
    void SetTitle(const std::string& title);
    const std::string& GetTitle() const { return title_; }

    // Update cell metrics after font loading (optional but recommended)
    void SetCellSize(int w, int h);

    // Resize the window so that it exactly fits the requested grid size.
    void ResizeToCells(int cols, int rows);

    // Poll next event. Returns true if event was retrieved.
    bool PollEvent(Event& out);

    // Cell dimensions
    int Cols() const { return cols_; }
    int Rows() const { return rows_; }
    int CellWidth() const { return cellWidth_; }
    int CellHeight() const { return cellHeight_; }

    // Pixel dimensions (uses the actual OpenGL drawable size for crisp rendering)
    int PixelWidth() const;
    int PixelHeight() const;

    // SDL window handle (for renderer use)
    void* GetNativeHandle() const; // returns SDL_Window*

    // Callback for resize (called internally when SDL_WINDOWEVENT_SIZE_CHANGED)
    using ResizeCallback = std::function<void(int newCols, int newRows)>;
    void SetResizeCallback(ResizeCallback cb) { onResize_ = std::move(cb); }

private:
    void RecalculateDimensions();

    SDL_Window* window_ = nullptr;
    void* glContext_ = nullptr;
    std::string title_;
    int cols_ = 0;
    int rows_ = 0;
    int fontSize_ = 16;
    int cellWidth_ = 0;
    int cellHeight_ = 0;
    int pixelWidth_ = 0;
    int pixelHeight_ = 0;
    ResizeCallback onResize_;
    std::vector<Event> pendingEvents_;
};

} // namespace bui
