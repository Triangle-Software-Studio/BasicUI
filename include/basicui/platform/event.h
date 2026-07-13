#pragma once

#include <cstdint>
#include <string>

namespace bui {

enum class EventType {
    // Keyboard
    KeyPress, KeyRelease,
    // Mouse
    MouseMove, MouseDown, MouseUp, MouseWheel,
    // Window
    Resize, Focus, Close,
    // None
    Unknown
};

enum class KeyCode {
    Unknown = 0,
    A = 'a', B, C, D, E, F, G, H, I, J, K, L, M,
    N, O, P, Q, R, S, T, U, V, W, X, Y, Z,
    Num0 = '0', Num1, Num2, Num3, Num4, Num5, Num6, Num7, Num8, Num9,
    Space = ' ', Enter = 13, Tab = 9, Backspace = 8, Escape = 27,
    Left = 256, Right, Up, Down,
    Home, End, PageUp, PageDown, Insert, Delete,
    F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12,
    LShift, RShift, LCtrl, RCtrl, LAlt, RAlt,
};

constexpr bool IsPrintable(KeyCode k) {
    auto v = static_cast<int>(k);
    return v >= 32 && v < 256;
}

struct KeyEvent {
    KeyCode key;
    int modifiers; // bit mask: 1=Shift, 2=Ctrl, 4=Alt
    char32_t text; // for text input events
};

struct MouseEvent {
    int x = 0; // cell coordinates
    int y = 0; // cell coordinates
    int button = 0; // 1=left, 2=middle, 3=right
    int delta = 0; // for wheel
};

struct ResizeEvent {
    int width = 0;  // new columns
    int height = 0; // new rows
};

struct Event {
    EventType type = EventType::Unknown;
    union {
        KeyEvent key;
        MouseEvent mouse;
        ResizeEvent resize;
    };

    Event() : type(EventType::Unknown) { key = {}; }
    explicit Event(EventType t) : type(t) { key = {}; }
    static Event Key(EventType t, KeyCode k, int mod = 0, char32_t txt = 0) {
        Event e;
        e.type = t;
        e.key = {k, mod, txt};
        return e;
    }
    static Event Mouse(EventType t, int mx, int my, int btn = 0, int d = 0) {
        Event e;
        e.type = t;
        e.mouse = {mx, my, btn, d};
        return e;
    }
    static Event Resize(int w, int h) {
        Event e;
        e.type = EventType::Resize;
        e.resize = {w, h};
        return e;
    }
};

} // namespace bui
