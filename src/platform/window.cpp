#include "basicui/platform/window.h"
#define NOMINMAX
#include <SDL.h>
#include <SDL_syswm.h>
#include <dwmapi.h>
#include <stdexcept>

namespace bui {

namespace {
#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif

void EnableDarkTitleBar(SDL_Window* window) {
    SDL_SysWMinfo info;
    SDL_VERSION(&info.version);
    if (!SDL_GetWindowWMInfo(window, &info)) return;
    if (info.subsystem == SDL_SYSWM_WINDOWS) {
        BOOL dark = TRUE;
        DwmSetWindowAttribute(info.info.win.window, DWMWA_USE_IMMERSIVE_DARK_MODE, &dark, sizeof(dark));
    }
}
} // namespace

Window::Window(const std::string& title, int cols, int rows, int fontSize)
    : title_(title), cols_(cols), rows_(rows), fontSize_(fontSize) {
    // Make the process DPI-aware so the OS does not bitmap-scale the window.
    SDL_SetHint(SDL_HINT_WINDOWS_DPI_AWARENESS, "permonitorv2");

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        throw std::runtime_error("SDL_Init failed");
    }

    // Approximate cell dimensions from font size (monospace heuristic)
    cellWidth_ = fontSize_ * 3 / 5;
    if (cellWidth_ < 1) cellWidth_ = 1;
    cellHeight_ = fontSize_;
    if (cellHeight_ < 1) cellHeight_ = 1;

    pixelWidth_ = cols_ * cellWidth_;
    pixelHeight_ = rows_ * cellHeight_;

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    window_ = SDL_CreateWindow(title_.c_str(),
                               SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                               pixelWidth_, pixelHeight_,
                               SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    if (!window_) {
        SDL_Quit();
        throw std::runtime_error("SDL_CreateWindow failed");
    }

    EnableDarkTitleBar(window_);

    glContext_ = SDL_GL_CreateContext(window_);
    if (!glContext_) {
        SDL_DestroyWindow(window_);
        SDL_Quit();
        throw std::runtime_error("SDL_GL_CreateContext failed");
    }
}

Window::~Window() {
    if (glContext_) {
        SDL_GL_DeleteContext(glContext_);
    }
    if (window_) {
        SDL_DestroyWindow(window_);
    }
    SDL_Quit();
}

bool Window::IsOpen() const {
    return window_ != nullptr;
}

void Window::Close() {
    if (glContext_) {
        SDL_GL_DeleteContext(glContext_);
        glContext_ = nullptr;
    }
    if (window_) {
        SDL_DestroyWindow(window_);
        window_ = nullptr;
    }
}

void Window::SetTitle(const std::string& title) {
    title_ = title;
    if (window_) {
        SDL_SetWindowTitle(window_, title_.c_str());
    }
}

bool Window::PollEvent(Event& out) {
    SDL_Event e;
    if (!SDL_PollEvent(&e)) {
        return false;
    }

    switch (e.type) {
        case SDL_QUIT:
            out = Event(EventType::Close);
            return true;

        case SDL_WINDOWEVENT:
            if (e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                pixelWidth_ = e.window.data1;
                pixelHeight_ = e.window.data2;
                RecalculateDimensions();
                out = Event::Resize(cols_, rows_);
                return true;
            }
            out = Event(EventType::Unknown);
            return true;

        case SDL_KEYDOWN:
        case SDL_KEYUP: {
            EventType t = (e.type == SDL_KEYDOWN) ? EventType::KeyPress : EventType::KeyRelease;
            KeyCode kc = KeyCode::Unknown;
            int mod = 0;
            if (e.key.keysym.mod & KMOD_SHIFT) mod |= 1;
            if (e.key.keysym.mod & KMOD_CTRL)  mod |= 2;
            if (e.key.keysym.mod & KMOD_ALT)   mod |= 4;

            switch (e.key.keysym.sym) {
                case SDLK_a: kc = KeyCode::A; break;
                case SDLK_b: kc = KeyCode::B; break;
                case SDLK_c: kc = KeyCode::C; break;
                case SDLK_d: kc = KeyCode::D; break;
                case SDLK_e: kc = KeyCode::E; break;
                case SDLK_f: kc = KeyCode::F; break;
                case SDLK_g: kc = KeyCode::G; break;
                case SDLK_h: kc = KeyCode::H; break;
                case SDLK_i: kc = KeyCode::I; break;
                case SDLK_j: kc = KeyCode::J; break;
                case SDLK_k: kc = KeyCode::K; break;
                case SDLK_l: kc = KeyCode::L; break;
                case SDLK_m: kc = KeyCode::M; break;
                case SDLK_n: kc = KeyCode::N; break;
                case SDLK_o: kc = KeyCode::O; break;
                case SDLK_p: kc = KeyCode::P; break;
                case SDLK_q: kc = KeyCode::Q; break;
                case SDLK_r: kc = KeyCode::R; break;
                case SDLK_s: kc = KeyCode::S; break;
                case SDLK_t: kc = KeyCode::T; break;
                case SDLK_u: kc = KeyCode::U; break;
                case SDLK_v: kc = KeyCode::V; break;
                case SDLK_w: kc = KeyCode::W; break;
                case SDLK_x: kc = KeyCode::X; break;
                case SDLK_y: kc = KeyCode::Y; break;
                case SDLK_z: kc = KeyCode::Z; break;
                case SDLK_0: kc = KeyCode::Num0; break;
                case SDLK_1: kc = KeyCode::Num1; break;
                case SDLK_2: kc = KeyCode::Num2; break;
                case SDLK_3: kc = KeyCode::Num3; break;
                case SDLK_4: kc = KeyCode::Num4; break;
                case SDLK_5: kc = KeyCode::Num5; break;
                case SDLK_6: kc = KeyCode::Num6; break;
                case SDLK_7: kc = KeyCode::Num7; break;
                case SDLK_8: kc = KeyCode::Num8; break;
                case SDLK_9: kc = KeyCode::Num9; break;
                case SDLK_SPACE:      kc = KeyCode::Space;     break;
                case SDLK_RETURN:     kc = KeyCode::Enter;     break;
                case SDLK_TAB:        kc = KeyCode::Tab;       break;
                case SDLK_BACKSPACE:  kc = KeyCode::Backspace; break;
                case SDLK_ESCAPE:     kc = KeyCode::Escape;    break;
                case SDLK_LEFT:       kc = KeyCode::Left;      break;
                case SDLK_RIGHT:      kc = KeyCode::Right;     break;
                case SDLK_UP:         kc = KeyCode::Up;        break;
                case SDLK_DOWN:       kc = KeyCode::Down;      break;
                case SDLK_HOME:       kc = KeyCode::Home;      break;
                case SDLK_END:        kc = KeyCode::End;       break;
                case SDLK_PAGEUP:     kc = KeyCode::PageUp;    break;
                case SDLK_PAGEDOWN:   kc = KeyCode::PageDown;  break;
                case SDLK_INSERT:     kc = KeyCode::Insert;    break;
                case SDLK_DELETE:     kc = KeyCode::Delete;    break;
                case SDLK_F1:         kc = KeyCode::F1;        break;
                case SDLK_F2:         kc = KeyCode::F2;        break;
                case SDLK_F3:         kc = KeyCode::F3;        break;
                case SDLK_F4:         kc = KeyCode::F4;        break;
                case SDLK_F5:         kc = KeyCode::F5;        break;
                case SDLK_F6:         kc = KeyCode::F6;        break;
                case SDLK_F7:         kc = KeyCode::F7;        break;
                case SDLK_F8:         kc = KeyCode::F8;        break;
                case SDLK_F9:         kc = KeyCode::F9;        break;
                case SDLK_F10:        kc = KeyCode::F10;       break;
                case SDLK_F11:        kc = KeyCode::F11;       break;
                case SDLK_F12:        kc = KeyCode::F12;       break;
                case SDLK_LSHIFT:     kc = KeyCode::LShift;    break;
                case SDLK_RSHIFT:     kc = KeyCode::RShift;    break;
                case SDLK_LCTRL:      kc = KeyCode::LCtrl;     break;
                case SDLK_RCTRL:      kc = KeyCode::RCtrl;     break;
                case SDLK_LALT:       kc = KeyCode::LAlt;      break;
                case SDLK_RALT:       kc = KeyCode::RAlt;      break;
                default:
                    if (e.key.keysym.sym >= SDLK_SPACE && e.key.keysym.sym < 128) {
                        kc = static_cast<KeyCode>(e.key.keysym.sym);
                    }
                    break;
            }

            char32_t txt = 0;
            if (kc >= KeyCode::Space && static_cast<int>(kc) < 256) {
                txt = static_cast<char32_t>(static_cast<int>(kc));
            }
            out = Event::Key(t, kc, mod, txt);
            return true;
        }

        case SDL_MOUSEMOTION: {
            int mx = e.motion.x / cellWidth_;
            int my = e.motion.y / cellHeight_;
            out = Event::Mouse(EventType::MouseMove, mx, my);
            return true;
        }

        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP: {
            EventType t = (e.type == SDL_MOUSEBUTTONDOWN) ? EventType::MouseDown : EventType::MouseUp;
            int mx = e.button.x / cellWidth_;
            int my = e.button.y / cellHeight_;
            int btn = 0;
            if (e.button.button == SDL_BUTTON_LEFT)   btn = 1;
            else if (e.button.button == SDL_BUTTON_MIDDLE) btn = 2;
            else if (e.button.button == SDL_BUTTON_RIGHT)  btn = 3;
            out = Event::Mouse(t, mx, my, btn);
            return true;
        }

        case SDL_MOUSEWHEEL: {
            int mx, my;
            SDL_GetMouseState(&mx, &my);
            int cx = mx / cellWidth_;
            int cy = my / cellHeight_;
            out = Event::Mouse(EventType::MouseWheel, cx, cy, 0, e.wheel.y);
            return true;
        }

        default:
            out = Event(EventType::Unknown);
            return true;
    }
}

void* Window::GetNativeHandle() const {
    return window_;
}

void Window::RecalculateDimensions() {
    if (cellWidth_ > 0 && cellHeight_ > 0) {
        cols_ = pixelWidth_ / cellWidth_;
        rows_ = pixelHeight_ / cellHeight_;
    }
    if (onResize_) {
        onResize_(cols_, rows_);
    }
}

void Window::SetCellSize(int w, int h) {
    if (w <= 0 || h <= 0) return;
    cellWidth_ = w;
    cellHeight_ = h;
    // Recalculate cols/rows from current pixel size
    if (window_) {
        RecalculateDimensions();
    }
}

void Window::ResizeToCells(int cols, int rows) {
    if (cols <= 0 || rows <= 0 || cellWidth_ <= 0 || cellHeight_ <= 0) return;
    cols_ = cols;
    rows_ = rows;
    pixelWidth_ = cols_ * cellWidth_;
    pixelHeight_ = rows_ * cellHeight_;
    if (window_) {
        SDL_SetWindowSize(window_, pixelWidth_, pixelHeight_);
        RecalculateDimensions();
    }
}

int Window::PixelWidth() const {
    if (!window_) return pixelWidth_;
    int w = 0, h = 0;
    SDL_GL_GetDrawableSize(window_, &w, &h);
    return w > 0 ? w : pixelWidth_;
}

int Window::PixelHeight() const {
    if (!window_) return pixelHeight_;
    int w = 0, h = 0;
    SDL_GL_GetDrawableSize(window_, &w, &h);
    return h > 0 ? h : pixelHeight_;
}

} // namespace bui
