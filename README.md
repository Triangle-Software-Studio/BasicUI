# BasicUI

[中文文档](README_CN.md) | English

A lightweight, terminal-inspired C++17 UI framework for Windows. Built with SDL2, OpenGL, and FreeType, it renders the entire interface on a character grid — like a modern terminal — but with GPU acceleration, rich widgets, and smooth animations.

> **Why BasicUI?** Traditional GUI frameworks are heavy. BasicUI gives you the simplicity of a terminal UI with the power of hardware-accelerated OpenGL rendering, all in a small footprint (~5-10 MB RAM).

---

## Table of Contents

- [Features](#features)
- [Requirements](#requirements)
- [Quick Start](#quick-start)
  - [One-Click Dependency Install](#one-click-dependency-install)
  - [Build with CMake + MinGW](#build-with-cmake--mingw-recommended)
  - [Build with Visual Studio](#build-with-visual-studio)
- [Running the Demo](#running-the-demo)
- [Usage Guide](#usage-guide)
  - [Hello World](#hello-world)
  - [Fluent API](#fluent-api)
  - [Multi-Font Support](#multi-font-support)
  - [Layout System](#layout-system)
  - [Event Handling](#event-handling)
  - [Theming](#theming)
- [Project Structure](#project-structure)
- [Architecture](#architecture)
- [Performance](#performance)
- [FAQ](#faq)
- [License](#license)

---

## Features

- **Grid-based layout** — All UI elements snap to a character grid, giving you perfect alignment without pixel math.
- **Rich widget set** — Label, Button, Input, Panel, Screen, ListView, MenuBar, ScrollBar, ComboBox, CheckBox, RadioButton, ProgressBar, TabControl, Tooltip.
- **Event-driven** — Full keyboard and mouse support with tab focus, hover states, and click handlers.
- **Hardware-accelerated rendering** — OpenGL instanced rendering + dirty-region tracking. Static frames cost near-zero CPU.
- **Multi-font support** — Register multiple fonts at different sizes. Switch per-widget or globally at runtime.
- **Fluent chainable API** — Write `SetBounds({...}).SetValue(50).ShowPercent(true)` instead of five separate lines.
- **Layered rendering** — Automatic z-index with `Render` and `RenderOverlay` passes. Dropdowns and tooltips naturally float above everything.
- **Panel without clipping** — Child controls can draw outside parent bounds, essential for menus and dropdowns.
- **Dark theme** — Windows 10/11 DWM dark title bar support out of the box.

---

## Requirements

| Component | Minimum Version |
|-----------|-----------------|
| OS | Windows 10 / Windows 11 |
| Compiler | GCC 9+ / MinGW-w64, or Visual Studio 2022 |
| CMake | 3.16+ |
| SDL2 | 2.x |
| FreeType | 2.x |
| GLEW | 2.x |

> **Dev-C++ users:** The code is standard C++17. Use **CMake + MinGW** (recommended) or configure a pure Dev-C++ project. Ensure you use MinGW versions of SDL2, FreeType, and GLEW. Add `-std=c++17 -DNOMINMAX` to compiler flags and link `-lSDL2 -lSDL2main -lfreetype -lglew32 -lopengl32 -ldwmapi`.

---

## Quick Start

### One-Click Dependency Install

If you don't have SDL2, FreeType, or GLEW installed, run the provided installer:

```bash
# MSVC (default)
install.bat

# MinGW
install.bat mingw
```

This will download all dependencies, configure CMake, build the project, and copy required DLLs automatically.

### Build with CMake + MinGW (Recommended)

```bash
# 1. Generate build files
cmake -B build -S . -G "MinGW Makefiles"

# 2. Build Release
cmake --build build --config Release

# 3. Copy runtime DLLs (if not using install.bat)
cp third_party/SDL2/lib/x64/SDL2.dll build/Release/
cp third_party/glew/bin/Release/x64/glew32.dll build/Release/
```

### Build with Visual Studio

```bash
cmake -B build -S . -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

---

## Running the Demo

```bash
cd build/Release
basicui_demo.exe [path-to-font.ttf]
```

If no font path is provided, it defaults to `fonts/CascadiaMonoNF-Regular.ttf` (relative to the working directory).

**Controls:**
- **Tab** — Switch focus between widgets.
- **Mouse click** — Interact with buttons, inputs, checkboxes, combo boxes, etc.
- **Type** — Enter text in the focused input box.
- **Enter** — Submit input or trigger button actions.
- **Arrow keys** — Navigate tabs, browse lists, move cursor within inputs.

---

## Usage Guide

### Hello World

```cpp
#include "basicui/widget/screen.h"
#include "basicui/widget/label.h"
#include "basicui/widget/button.h"
#include "basicui/render/text_grid.h"

using namespace bui;

auto screen = std::make_shared<Screen>();
auto label  = std::make_shared<Label>("Hello, BasicUI!");
label->SetBounds({2, 2, 20, 1});

auto button = std::make_shared<Button>("Click Me");
button->SetBounds({2, 4, 12, 1});
button->OnClick([]() {
    std::cout << "Button clicked!\n";
});

screen->AddChild(label);
screen->AddChild(button);
```

### Fluent API

Most setters return a reference to `*this`, enabling chainable configuration:

```cpp
auto input = std::make_shared<Input>();
input->SetBounds({2, 2, 30, 1})
      .SetPlaceholder("Enter your name...")
      .SetMaxLength(32);

auto progress = std::make_shared<ProgressBar>();
progress->SetBounds({2, 6, 20, 1})
         .Range(0, 100)
         .Value(75)
         .ShowPercent(true);
```

### Multi-Font Support

```cpp
// Register fonts at different sizes
FontManager::Instance().Register("default", "fonts/CascadiaMonoNF-Regular.ttf", 24);
FontManager::Instance().Register("small",   "fonts/CascadiaMonoNF-Regular.ttf", 20);

// Use in renderer
renderer.SetGlyphAtlas(FontManager::Instance().GetAtlas("default"));
renderer.SetFontAtlas("small", FontManager::Instance().GetAtlas("small"));

// Assign font to a specific widget
titleLabel->SetFont("small");
```

### Layout System

BasicUI supports manual bounds placement and automatic layout via `FlexLayout`:

```cpp
auto panel = std::make_shared<Panel>();
panel->SetLayout(std::make_shared<FlexLayout>(FlexLayout::Vertical, 1, 1));

// Children are arranged automatically
panel->AddChild(button1);
panel->AddChild(button2);
panel->AddChild(button3);
```

### Event Handling

Events bubble from `Window` → `Screen` → target widget. Implement `OnEvent` to handle input:

```cpp
class MyWidget : public Widget {
public:
    void OnEvent(const Event& ev) override {
        if (ev.type == EventType::MouseDown) {
            // Handle click at (ev.mouse.x, ev.mouse.y)
        }
    }
};
```

### Theming

Colors are defined in `style/theme.h`. Override individual colors or replace the entire theme:

```cpp
// Custom button color
button->SetFgColor(MakeColor(0, 255, 128, 255));   // Bright green text
button->SetBgColor(MakeColor(24, 24, 24, 255));    // Dark background
```

---

## Project Structure

```
BasicUI/
├── CMakeLists.txt          # CMake build configuration
├── LICENSE.txt             # MIT License
├── README.md               # This file
├── README_CN.md            # Chinese documentation
├── install.bat             # One-click dependency installer (Windows)
├── install.ps1             # PowerShell backend for install.bat
├── demo/
│   └── main.cpp            # Widget Gallery demo application
├── fonts/
│   └── CascadiaMonoNF-Regular.ttf
├── include/basicui/        # Public headers
│   ├── basicui.h           # Core types: Color, Point, Rect
│   ├── layout/             # FlexLayout, GridLayout
│   ├── platform/           # Window, Event system
│   ├── render/             # Renderer, TextGrid, GlyphAtlas, Shader
│   ├── style/              # Theme, FontManager
│   └── widget/             # Widget base + all controls
└── src/                    # Implementation files
    ├── layout/
    ├── platform/
    ├── render/
    ├── style/
    └── widget/
```

---

## Architecture

```
┌─────────────────────────────────────┐
│  Window (SDL2 + OpenGL context)     │
├─────────────────────────────────────┤
│  Renderer                           │
│  ├─ GlyphAtlas (FreeType → OpenGL)  │
│  ├─ Shader (instanced quad)         │
│  └─ Dirty-region cache              │
├─────────────────────────────────────┤
│  Screen                             │
│  ├─ Widget tree (z-index sorted)    │
│  ├─ Focus management                │
│  └─ Event dispatch                  │
├─────────────────────────────────────┤
│  TextGrid (2D cell array)           │
│  ├─ Unicode codepoint per cell      │
│  ├─ fg/bg Color                     │
│  ├─ Style flags (bold, underline)   │
│  └─ Font index                      │
└─────────────────────────────────────┘
```

- **Window** — Creates the SDL2 window with an OpenGL context and DWM dark title bar.
- **Renderer** — Converts the `TextGrid` into OpenGL instanced draw calls. Only uploads GPU data when the grid changes (dirty-region tracking).
- **GlyphAtlas** — Renders glyph bitmaps with FreeType, embeds them into cell-sized canvases, and packs them into an OpenGL texture atlas. Prevents narrow glyphs from stretching.
- **Screen** — Root widget that collects all visible widgets, sorts them by z-index, and performs two-pass rendering (`Render` → `RenderOverlay`).
- **FontManager** — Singleton that manages multiple `GlyphAtlas` instances. Each font registration creates its own atlas, and the renderer batches cells by font index to minimize texture binds.

---

## Performance

| Metric | Value |
|--------|-------|
| RAM usage (typical) | ~5–10 MB |
| Draw calls per frame | 1–N (one per unique font) |
| Static frame CPU cost | Near zero (dirty-region skip) |
| VBO strategy | Pre-allocated + 2x dynamic growth |
| Atlas texture size | ~512×512 per font (RGBA) |

Key optimizations:
- **Instanced rendering** — One `glDrawArraysInstanced` call renders all cells.
- **Dirty-region tracking** — If the grid hasn't changed since last frame, no GPU upload occurs.
- **Multi-font batching** — Cells are grouped by font index; each font gets one draw call.
- **2x VBO growth** — Buffer reallocations are exponentially less frequent as the UI grows.

---

## FAQ

**Q: Can I use BasicUI on Linux or macOS?**
A: Not out of the box. The core code (SDL2, OpenGL, FreeType) is cross-platform, but `src/platform/window.cpp` uses Windows-specific `dwmapi` for the dark title bar. Porting requires wrapping that code in `#ifdef _WIN32`. The CMake `-ldwmapi` link flag also needs to be conditional.

**Q: The program says "Failed to load font"?**
A: Ensure `fonts/CascadiaMonoNF-Regular.ttf` exists in the working directory, or pass the correct path as a command-line argument.

**Q: Missing `SDL2.dll` or `glew32.dll`?**
A: Copy these DLLs from your dependency directory to the same folder as `basicui_demo.exe`. The `install.bat` script does this automatically.

**Q: Text looks blurry or stretched?**
A: Use a monospace font. In `demo/main.cpp`, make sure the font size matches the cell metrics calculated by `GlyphAtlas`.

**Q: Can I change the font size at runtime?**
A: Yes. Call `GlyphAtlas::SetPixelHeight(newSize)` and the atlas will regenerate. Re-register the font via `FontManager::Instance().Register("name", path, newSize)` if you need multiple sizes simultaneously.

**Q: How do I create a custom widget?**
A: Inherit from `bui::Widget`, override `OnRender(TextGrid&)` to draw your cells, and optionally `OnEvent(const Event&)` to handle input.

---

## License

[MIT License](LICENSE.txt)
