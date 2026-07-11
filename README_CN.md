# BasicUI

[English](README.md) | 中文文档

一个受终端启发的轻量级 C++17 Windows UI 框架。基于 SDL2、OpenGL 和 FreeType 构建，将整个界面渲染在字符网格上——如同现代终端——但拥有 GPU 加速、丰富的控件和流畅的动画。

> **为什么选择 BasicUI？** 传统 GUI 框架过于笨重。BasicUI 在仅约 5–10 MB 内存占用的前提下，为你带来终端 UI 的简洁与硬件加速 OpenGL 渲染的强大能力。

---

## 目录

- [功能特点](#功能特点)
- [环境要求](#环境要求)
- [快速开始](#快速开始)
  - [一键安装依赖](#一键安装依赖)
  - [CMake + MinGW 构建（推荐）](#cmake--mingw-构建推荐)
  - [Visual Studio 构建](#visual-studio-构建)
- [运行演示程序](#运行演示程序)
- [使用指南](#使用指南)
  - [Hello World](#hello-world)
  - [流式 API](#流式-api)
  - [多字体支持](#多字体支持)
  - [布局系统](#布局系统)
  - [事件处理](#事件处理)
  - [主题定制](#主题定制)
- [项目结构](#项目结构)
- [核心架构](#核心架构)
- [性能表现](#性能表现)
- [常见问题](#常见问题)
- [许可证](#许可证)

---

## 功能特点

- **网格布局** —— 所有 UI 元素自动对齐到字符网格，无需像素级计算即可实现完美对齐。
- **丰富控件集** —— 标签（Label）、按钮（Button）、输入框（Input）、面板（Panel）、屏幕（Screen）、列表（ListView）、菜单栏（MenuBar）、滚动条（ScrollBar）、下拉框（ComboBox）、复选框（CheckBox）、单选框（RadioButton）、进度条（ProgressBar）、选项卡（TabControl）、提示框（Tooltip）。
- **事件驱动** —— 完整的键盘和鼠标支持，包含 Tab 焦点切换、悬停状态和点击回调。
- **硬件加速渲染** —— OpenGL 实例化渲染 + 脏区域检测。静态界面的 CPU 开销接近零。
- **多字体支持** —— 注册不同字号的多种字体，支持按控件切换或运行时全局调整。
- **流式链式 API** —— 用 `SetBounds({...}).SetValue(50).ShowPercent(true)` 替代五行独立代码。
- **层级渲染** —— 通过 `Render` 和 `RenderOverlay` 两道渲染自动处理 z-index，下拉菜单和提示框可自然浮于顶层。
- **无裁剪面板** —— 子控件可绘制在父控件边界之外，这对菜单和下拉框至关重要。
- **深色主题** —— 开箱即支持 Windows 10/11 DWM 深色标题栏。

---

## 环境要求

| 组件 | 最低版本 |
|------|---------|
| 操作系统 | Windows 10 / Windows 11 |
| 编译器 | GCC 9+ / MinGW-w64，或 Visual Studio 2022 |
| CMake | 3.16+ |
| SDL2 | 2.x |
| FreeType | 2.x |
| GLEW | 2.x |

> **Dev-C++ 用户：** 本框架代码为标准 C++17。推荐使用 **CMake + MinGW** 构建；也可手动配置纯 Dev-C++ 项目。编译参数需加 `-std=c++17 -DNOMINMAX`，链接库为 `-lSDL2 -lSDL2main -lfreetype -lglew32 -lopengl32 -ldwmapi`。依赖库必须是 MinGW 版本（MSVC 的 .lib 不兼容）。

---

## 快速开始

### 一键安装依赖

如果你尚未安装 SDL2、FreeType 或 GLEW，可运行提供的一键安装脚本：

```bash
# MSVC（默认）
install.bat

# MinGW
install.bat mingw
```

此脚本将自动下载所有依赖、配置 CMake、构建项目并复制所需的 DLL。

### CMake + MinGW 构建（推荐）

```bash
# 1. 生成构建文件
cmake -B build -S . -G "MinGW Makefiles"

# 2. 构建 Release 版本
cmake --build build --config Release

# 3. 复制运行时 DLL（如未使用 install.bat）
cp third_party/SDL2/lib/x64/SDL2.dll build/Release/
cp third_party/glew/bin/Release/x64/glew32.dll build/Release/
```

### Visual Studio 构建

```bash
cmake -B build -S . -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
```

---

## 运行演示程序

```bash
cd build/Release
basicui_demo.exe [字体路径]
```

如果未指定字体路径，程序默认使用 `fonts/CascadiaMonoNF-Regular.ttf`（相对于工作目录）。

**操作说明：**
- **Tab** —— 在控件之间切换焦点。
- **鼠标点击** —— 与按钮、输入框、复选框、下拉框等交互。
- **打字** —— 在获得焦点的输入框中输入文字。
- **Enter** —— 提交输入或触发按钮动作。
- **方向键** —— 切换选项卡、浏览列表、在输入框中移动光标。

---

## 使用指南

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

### 流式 API

大部分 setter 返回 `*this` 的引用，支持链式配置：

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

### 多字体支持

```cpp
// 注册不同字号的字体
FontManager::Instance().Register("default", "fonts/CascadiaMonoNF-Regular.ttf", 24);
FontManager::Instance().Register("small",   "fonts/CascadiaMonoNF-Regular.ttf", 20);

// 在渲染器中使用
renderer.SetGlyphAtlas(FontManager::Instance().GetAtlas("default"));
renderer.SetFontAtlas("small", FontManager::Instance().GetAtlas("small"));

// 为特定控件指定字体
titleLabel->SetFont("small");
```

### 布局系统

BasicUI 支持手动边界放置和通过 `FlexLayout` 自动布局：

```cpp
auto panel = std::make_shared<Panel>();
panel->SetLayout(std::make_shared<FlexLayout>(FlexLayout::Vertical, 1, 1));

// 子控件自动排列
panel->AddChild(button1);
panel->AddChild(button2);
panel->AddChild(button3);
```

### 事件处理

事件从 `Window` → `Screen` → 目标控件冒泡。重写 `OnEvent` 来处理输入：

```cpp
class MyWidget : public Widget {
public:
    void OnEvent(const Event& ev) override {
        if (ev.type == EventType::MouseDown) {
            // 处理 (ev.mouse.x, ev.mouse.y) 处的点击
        }
    }
};
```

### 主题定制

颜色定义在 `style/theme.h` 中。可单独覆盖颜色或替换整个主题：

```cpp
// 自定义按钮颜色
button->SetFgColor(MakeColor(0, 255, 128, 255));   // 亮绿色文字
button->SetBgColor(MakeColor(24, 24, 24, 255));    // 深色背景
```

---

## 项目结构

```
BasicUI/
├── CMakeLists.txt          # CMake 构建配置
├── LICENSE.txt             # MIT 许可证
├── README.md               # 英文文档
├── README_CN.md            # 本文档
├── install.bat             # 一键依赖安装脚本（Windows）
├── install.ps1             # install.bat 的 PowerShell 后端
├── demo/
│   └── main.cpp            # Widget Gallery 演示程序
├── fonts/
│   └── CascadiaMonoNF-Regular.ttf
├── include/basicui/        # 公共头文件
│   ├── basicui.h           # 核心类型：Color、Point、Rect
│   ├── layout/             # FlexLayout、GridLayout
│   ├── platform/           # 窗口、事件系统
│   ├── render/             # 渲染器、文本网格、字形图集、着色器
│   ├── style/              # 主题、字体管理
│   └── widget/             # 控件基类及所有控件
└── src/                    # 实现源文件
    ├── layout/
    ├── platform/
    ├── render/
    ├── style/
    └── widget/
```

---

## 核心架构

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

- **Window** —— 创建 SDL2 窗口，附带 OpenGL 上下文和 DWM 深色标题栏。
- **Renderer** —— 将 `TextGrid` 转换为 OpenGL 实例化绘制调用。仅在网格发生变化时上传 GPU 数据（脏区域检测）。
- **GlyphAtlas** —— 使用 FreeType 渲染字形位图，先将其嵌入固定大小的 canvas，再打包到 OpenGL 纹理图集。防止窄字符被拉伸变形。
- **Screen** —— 根控件，收集所有可见控件，按 z-index 排序，并执行两道渲染（`Render` → `RenderOverlay`）。
- **FontManager** —— 单例管理多个 `GlyphAtlas` 实例。每次注册字体都会创建独立的图集，渲染器按字体索引对单元格分批处理，以最小化纹理绑定次数。

---

## 性能表现

| 指标 | 数值 |
|------|------|
| 典型内存占用 | ~5–10 MB |
| 每帧绘制调用 | 1–N（每种独立字体一次） |
| 静态帧 CPU 开销 | 接近零（脏区域跳过） |
| VBO 策略 | 预分配 + 2 倍动态扩容 |
| 图集纹理大小 | 每种字体约 512×512（RGBA） |

关键优化点：
- **实例化渲染** —— 一次 `glDrawArraysInstanced` 调用渲染所有单元格。
- **脏区域检测** —— 若网格自上一帧未发生变化，则不进行 GPU 上传。
- **多字体批处理** —— 按字体索引对单元格分组；每种字体仅需一次绘制调用。
- **2 倍 VBO 扩容** —— 随着 UI 规模增长，缓冲区重新分配的频率指数级降低。

---

## 常见问题

**Q: BasicUI 能在 Linux 或 macOS 上使用吗？**
A: 不能直接使用。核心代码（SDL2、OpenGL、FreeType）是跨平台的，但 `src/platform/window.cpp` 使用了 Windows 专属的 `dwmapi` 来实现深色标题栏。移植时需要将该代码包裹在 `#ifdef _WIN32` 中。CMake 中的 `-ldwmapi` 链接标志也需要改为条件添加。

**Q: 程序提示 "Failed to load font"？**
A: 确保 `fonts/CascadiaMonoNF-Regular.ttf` 存在于工作目录中，或通过命令行参数传入正确的字体路径。

**Q: 缺少 `SDL2.dll` 或 `glew32.dll`？**
A: 从依赖库目录中将这两个 DLL 复制到 `basicui_demo.exe` 所在目录。`install.bat` 脚本会自动完成此操作。

**Q: 文字显示模糊或拉伸变形？**
A: 请使用等宽字体（monospace）。在 `demo/main.cpp` 中，确保字号与 `GlyphAtlas` 计算的单元格指标相匹配。

**Q: 可以在运行时修改字号吗？**
A: 可以。调用 `GlyphAtlas::SetPixelHeight(newSize)` 即可重新生成图集。若需同时使用多种字号，可通过 `FontManager::Instance().Register("name", path, newSize)` 重新注册字体。

**Q: 如何创建自定义控件？**
A: 继承 `bui::Widget`，重写 `OnRender(TextGrid&)` 来绘制单元格，并可选重写 `OnEvent(const Event&)` 来处理输入。

---

## 许可证

[MIT License](LICENSE.txt)
