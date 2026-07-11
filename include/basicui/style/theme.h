#pragma once

#include "basicui/basicui.h"

namespace bui {

namespace Theme {

// Windows 10 / Windows Terminal inspired dark console colors
constexpr Color DefaultBg       = MakeColor(12, 12, 12, 255);       // #0C0C0C
constexpr Color DefaultFg       = MakeColor(204, 204, 204, 255);   // #CCCCCC
constexpr Color TitleBarActive  = MakeColor(31, 31, 31, 255);      // #1F1F1F
constexpr Color TitleBarInactive= MakeColor(20, 20, 20, 255);      // #141414
constexpr Color BorderActive    = MakeColor(128, 128, 128, 255);   // #808080
constexpr Color BorderInactive  = MakeColor(96, 96, 96, 255);      // #606060
constexpr Color SelectionBg     = MakeColor(0, 55, 218, 255);      // #0037DA
constexpr Color SelectionFg     = MakeColor(255, 255, 255, 255);   // #FFFFFF
constexpr Color ButtonBg        = MakeColor(48, 48, 48, 255);      // #303030
constexpr Color ButtonFg        = MakeColor(204, 204, 204, 255);   // #CCCCCC
constexpr Color ButtonHoverBg   = MakeColor(64, 64, 64, 255);      // #404040
constexpr Color ButtonPressedBg = MakeColor(32, 32, 32, 255);      // #202020
constexpr Color ScrollBarBg     = MakeColor(60, 60, 60, 255);      // #3C3C3C
constexpr Color ScrollBarThumb  = MakeColor(120, 120, 120, 255);    // #787878
constexpr Color ScrollBarThumbHover = MakeColor(160, 160, 160, 255); // #A0A0A0
constexpr Color PanelBorder     = MakeColor(128, 128, 128, 255);   // #808080
constexpr Color MenuBarBg       = MakeColor(12, 12, 12, 255);      // #0C0C0C
constexpr Color MenuHighlightBg = MakeColor(0, 55, 218, 255);      // #0037DA

} // namespace Theme
} // namespace bui

