#include "basicui/render/renderer.h"
#include "basicui/render/text_grid.h"
#include "basicui/render/glyph_atlas.h"
#include "basicui/render/shader.h"
#include "basicui/platform/window.h"
#include "basicui/style/theme.h"
#include <GL/glew.h>
#define NOMINMAX
#include <SDL.h>
#include <vector>
#include <algorithm>
#include <unordered_map>
#include <string>

namespace bui {

struct CellInstance {
    float colRow[2];
    float color[4];
    float bgColor[4];
    float uv[4];
    float glyphScale[2];
    float glyphOffset[2];
};

struct Renderer::Impl {
    Window* window = nullptr;
    std::shared_ptr<GlyphAtlas> atlas;
    std::unordered_map<std::string, std::shared_ptr<GlyphAtlas>> fontAtlases;
    Shader shader;

    GLuint quadVAO = 0;
    GLuint quadVBO = 0;
    GLuint instanceVBO = 0;
    size_t instanceBufferSize_ = 0;

    std::vector<CellInstance> instances;
    std::vector<CellInstance> decoInstances;

    int titleBarRows_ = 0;
    int contentOffsetX_ = 0;
    int contentOffsetY_ = 0;

    // Simple dirty-region tracking: if grid content is identical to last frame, skip rebuilding instances.
    std::vector<Cell> lastCells;
    int lastGridW = 0;
    int lastGridH = 0;
    bool gridDirty = true;
};

Renderer::Renderer() : impl_(std::make_unique<Impl>()) {}

Renderer::~Renderer() {
    if (impl_->quadVAO) glDeleteVertexArrays(1, &impl_->quadVAO);
    if (impl_->quadVBO) glDeleteBuffers(1, &impl_->quadVBO);
    if (impl_->instanceVBO) glDeleteBuffers(1, &impl_->instanceVBO);
}

bool Renderer::Initialize(Window* window) {
    impl_->window = window;

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        return false;
    }
    const char* vertexSrc = R"(
        #version 330 core
        layout (location = 0) in vec2 aPos;
        layout (location = 1) in vec2 aUV;
        layout (location = 2) in vec2 iColRow;
        layout (location = 3) in vec4 iColor;
        layout (location = 4) in vec4 iBgColor;
        layout (location = 5) in vec4 iUV;
        layout (location = 6) in vec2 iGlyphScale;
        layout (location = 7) in vec2 iGlyphOffset;

        uniform vec2 uCellSize;
        uniform vec2 uWindowSize;

        out vec2 vUV;
        out vec4 vColor;
        out vec4 vBgColor;

        void main() {
            vec2 pos = aPos * iGlyphScale + iGlyphOffset;
            vec2 pixelPos = (pos + iColRow) * uCellSize;
            // Map grid coordinates (origin at top-left) to NDC (origin at bottom-left).
            pixelPos.y = uWindowSize.y - pixelPos.y;
            vec2 ndc = (pixelPos / uWindowSize) * 2.0 - 1.0;
            gl_Position = vec4(ndc, 0.0, 1.0);
            vec2 uv = mix(iUV.xy, iUV.zw, aUV);
            vUV = uv;
            vColor = iColor;
            vBgColor = iBgColor;
        }
    )";

    const char* fragmentSrc = R"(
        #version 330 core
        in vec2 vUV;
        in vec4 vColor;
        in vec4 vBgColor;

        uniform sampler2D uAtlas;

        out vec4 FragColor;

        void main() {
            float alpha = texture(uAtlas, vUV).r;
            // Pre-multiplied alpha blending: glyph alpha modulates foreground over background.
            vec4 fg = vColor * alpha;
            FragColor = fg + vBgColor * (1.0 - fg.a);
        }
    )";

    if (!impl_->shader.Load(vertexSrc, fragmentSrc)) {
        return false;
    }

    // Build quad: 4 vertices, triangle strip
    float quad[] = {
        // pos, uv
        0.0f, 0.0f, 0.0f, 0.0f,
        1.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 1.0f,
        1.0f, 1.0f, 1.0f, 1.0f,
    };

    glGenVertexArrays(1, &impl_->quadVAO);
    glBindVertexArray(impl_->quadVAO);

    glGenBuffers(1, &impl_->quadVBO);
    glBindBuffer(GL_ARRAY_BUFFER, impl_->quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

    glGenBuffers(1, &impl_->instanceVBO);
    glBindBuffer(GL_ARRAY_BUFFER, impl_->instanceVBO);
    impl_->instanceBufferSize_ = sizeof(CellInstance) * 4096;
    glBufferData(GL_ARRAY_BUFFER, impl_->instanceBufferSize_, nullptr, GL_DYNAMIC_DRAW);

    // Instance attributes
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(CellInstance), (void*)offsetof(CellInstance, colRow));
    glVertexAttribDivisor(2, 1);

    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(CellInstance), (void*)offsetof(CellInstance, color));
    glVertexAttribDivisor(3, 1);

    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(CellInstance), (void*)offsetof(CellInstance, bgColor));
    glVertexAttribDivisor(4, 1);

    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, sizeof(CellInstance), (void*)offsetof(CellInstance, uv));
    glVertexAttribDivisor(5, 1);

    glEnableVertexAttribArray(6);
    glVertexAttribPointer(6, 2, GL_FLOAT, GL_FALSE, sizeof(CellInstance), (void*)offsetof(CellInstance, glyphScale));
    glVertexAttribDivisor(6, 1);

    glEnableVertexAttribArray(7);
    glVertexAttribPointer(7, 2, GL_FLOAT, GL_FALSE, sizeof(CellInstance), (void*)offsetof(CellInstance, glyphOffset));
    glVertexAttribDivisor(7, 1);

    glBindVertexArray(0);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    return true;
}

void Renderer::Shutdown() {
    if (impl_->quadVAO) {
        glDeleteVertexArrays(1, &impl_->quadVAO);
        impl_->quadVAO = 0;
    }
    if (impl_->quadVBO) {
        glDeleteBuffers(1, &impl_->quadVBO);
        impl_->quadVBO = 0;
    }
    if (impl_->instanceVBO) {
        glDeleteBuffers(1, &impl_->instanceVBO);
        impl_->instanceVBO = 0;
    }
    impl_->window = nullptr;
    impl_->atlas.reset();
}

void Renderer::SetGlyphAtlas(std::shared_ptr<GlyphAtlas> atlas) {
    impl_->atlas = std::move(atlas);
}

void Renderer::SetFontAtlas(const std::string& name, std::shared_ptr<GlyphAtlas> atlas) {
    if (name.empty() || name == "default") {
        impl_->atlas = std::move(atlas);
    } else {
        impl_->fontAtlases[name] = std::move(atlas);
    }
}

void Renderer::Render(const TextGrid& grid, int gridCols, int gridRows) {
    if (!impl_->window || !impl_->atlas) return;

    int defaultCellW = impl_->atlas->CellWidth();
    int defaultCellH = impl_->atlas->CellHeight();
    int winW = impl_->window->PixelWidth();
    int winH = impl_->window->PixelHeight();

    glViewport(0, 0, winW, winH);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    impl_->shader.Use();
    impl_->shader.SetVec2("uWindowSize", static_cast<float>(winW), static_cast<float>(winH));
    impl_->shader.SetInt("uAtlas", 0);

    int w = std::min(grid.Width(), gridCols);
    int h = std::min(grid.Height(), gridRows);

    float offsetX = static_cast<float>(impl_->contentOffsetX_);
    float offsetY = static_cast<float>(impl_->contentOffsetY_);

    const auto& cells = grid.Cells();
    const auto& fontNames = grid.FontNames();

    // Determine whether the grid uses any non-default font.
    bool hasNonDefaultFont = false;
    for (int i = 0; i < w * h && !hasNonDefaultFont; ++i) {
        if (cells[i].font != 0) hasNonDefaultFont = true;
    }
    bool hasDeco = !impl_->decoInstances.empty();

    // Dirty check: if grid content is identical to last frame, reuse previous draw command.
    bool gridChanged = impl_->gridDirty ||
                       impl_->lastGridW != w || impl_->lastGridH != h ||
                       impl_->lastCells.size() != static_cast<size_t>(w * h) ||
                       hasNonDefaultFont || hasDeco;
    if (!gridChanged) {
        for (int i = 0; i < w * h; ++i) {
            const Cell& a = cells[i];
            const Cell& b = impl_->lastCells[i];
            if (a.codepoint != b.codepoint || a.fg != b.fg || a.bg != b.bg || a.flags != b.flags || a.font != b.font) {
                gridChanged = true;
                break;
            }
        }
    }

    // Fast path: only the default font is used and grid hasn't changed.
    if (!gridChanged) {
        if (impl_->instanceBufferSize_ > 0) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, impl_->atlas->TextureId());
            impl_->shader.SetVec2("uCellSize", static_cast<float>(defaultCellW), static_cast<float>(defaultCellH));
            glBindVertexArray(impl_->quadVAO);
            glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, static_cast<GLsizei>(impl_->lastGridW * impl_->lastGridH));
            glBindVertexArray(0);
        }
        impl_->decoInstances.clear();
        impl_->contentOffsetX_ = 0;
        impl_->contentOffsetY_ = 0;
        impl_->titleBarRows_ = 0;
        return;
    }

    auto resolveAtlas = [&](uint16_t fontIdx) -> std::shared_ptr<GlyphAtlas> {
        const std::string& fontName = (fontIdx < fontNames.size()) ? fontNames[fontIdx] : std::string();
        if (!fontName.empty()) {
            auto it = impl_->fontAtlases.find(fontName);
            if (it != impl_->fontAtlases.end() && it->second) return it->second;
        }
        return impl_->atlas;
    };

    // Group instances by font index.
    std::unordered_map<uint16_t, std::vector<CellInstance>> batches;

    auto addCell = [&](const Cell& cell, int x, int y) {
        uint16_t fontIdx = cell.font;
        auto atlas = resolveAtlas(fontIdx);
        const GlyphInfo* glyph = atlas->GetGlyph(cell.codepoint);
        bool missing = false;
        if (!glyph) {
            glyph = atlas->GetGlyph(' ');
            missing = true;
        }
        if (!glyph) return;

        CellInstance inst;
        inst.colRow[0] = static_cast<float>(x) + offsetX;
        inst.colRow[1] = static_cast<float>(y) + offsetY;
        if (missing) {
            // Missing glyph: render nothing but keep the background cell.
            inst.color[0] = ColorR(cell.bg) / 255.0f;
            inst.color[1] = ColorG(cell.bg) / 255.0f;
            inst.color[2] = ColorB(cell.bg) / 255.0f;
            inst.color[3] = 0.0f;
        } else {
            inst.color[0] = ColorR(cell.fg) / 255.0f;
            inst.color[1] = ColorG(cell.fg) / 255.0f;
            inst.color[2] = ColorB(cell.fg) / 255.0f;
            inst.color[3] = ColorA(cell.fg) / 255.0f;
        }
        inst.bgColor[0] = ColorR(cell.bg) / 255.0f;
        inst.bgColor[1] = ColorG(cell.bg) / 255.0f;
        inst.bgColor[2] = ColorB(cell.bg) / 255.0f;
        inst.bgColor[3] = ColorA(cell.bg) / 255.0f;
        inst.uv[0] = glyph->u0;
        inst.uv[1] = glyph->v0;
        inst.uv[2] = glyph->u1;
        inst.uv[3] = glyph->v1;

        // Scale glyph to match the default grid cell size. The cell position is still
        // based on the default atlas so that mixed fonts stay aligned on the grid.
        float scaleX = static_cast<float>(atlas->CellWidth()) / static_cast<float>(defaultCellW);
        float scaleY = static_cast<float>(atlas->CellHeight()) / static_cast<float>(defaultCellH);
        inst.glyphScale[0] = scaleX;
        inst.glyphScale[1] = scaleY;
        inst.glyphOffset[0] = 0.0f;
        inst.glyphOffset[1] = 0.0f;

        batches[fontIdx].push_back(inst);
    };

    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            addCell(cells[y * grid.Width() + x], x, y);
        }
    }

    // Decoration instances always use the default font.
    if (!impl_->decoInstances.empty()) {
        batches[0].insert(batches[0].end(), impl_->decoInstances.begin(), impl_->decoInstances.end());
    }

    // Render each font batch.
    glBindVertexArray(impl_->quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, impl_->instanceVBO);

    for (auto& pair : batches) {
        uint16_t fontIdx = pair.first;
        auto& instances = pair.second;
        if (instances.empty()) continue;

        auto atlas = resolveAtlas(fontIdx);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, atlas->TextureId());
        impl_->shader.SetVec2("uCellSize", static_cast<float>(defaultCellW), static_cast<float>(defaultCellH));

        size_t needed = instances.size() * sizeof(CellInstance);
        if (needed > impl_->instanceBufferSize_) {
            impl_->instanceBufferSize_ = needed * 2;
            glBufferData(GL_ARRAY_BUFFER, impl_->instanceBufferSize_, nullptr, GL_DYNAMIC_DRAW);
        }
        glBufferSubData(GL_ARRAY_BUFFER, 0, needed, instances.data());

        glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, static_cast<GLsizei>(instances.size()));
    }

    glBindVertexArray(0);

    // Cache last grid content for next frame dirty check
    impl_->lastCells.assign(cells.begin(), cells.begin() + w * h);
    impl_->lastGridW = w;
    impl_->lastGridH = h;

    impl_->decoInstances.clear();
    impl_->contentOffsetX_ = 0;
    impl_->contentOffsetY_ = 0;
    impl_->titleBarRows_ = 0;
    impl_->gridDirty = false;
}

void Renderer::DrawTitleBar(const std::string& title, int cols, bool active, int& outTitleBarRows) {
    outTitleBarRows = 1;
    impl_->titleBarRows_ = 1;
    if (!impl_->atlas) return;

    Color bg = active ? Theme::TitleBarActive : Theme::TitleBarInactive;
    Color fg = active ? Theme::DefaultFg : Theme::DefaultFg;

    const GlyphInfo* spaceGlyph = impl_->atlas->GetGlyph(' ');
    if (!spaceGlyph) return;

    for (int x = 0; x < cols; ++x) {
        CellInstance inst;
        inst.colRow[0] = static_cast<float>(x);
        inst.colRow[1] = 0.0f;
        inst.color[0] = ColorR(fg) / 255.0f;
        inst.color[1] = ColorG(fg) / 255.0f;
        inst.color[2] = ColorB(fg) / 255.0f;
        inst.color[3] = ColorA(fg) / 255.0f;
        inst.bgColor[0] = ColorR(bg) / 255.0f;
        inst.bgColor[1] = ColorG(bg) / 255.0f;
        inst.bgColor[2] = ColorB(bg) / 255.0f;
        inst.bgColor[3] = ColorA(bg) / 255.0f;
        inst.uv[0] = spaceGlyph->u0;
        inst.uv[1] = spaceGlyph->v0;
        inst.uv[2] = spaceGlyph->u1;
        inst.uv[3] = spaceGlyph->v1;
        inst.glyphScale[0] = 1.0f;
        inst.glyphScale[1] = 1.0f;
        inst.glyphOffset[0] = 0.0f;
        inst.glyphOffset[1] = 0.0f;
        impl_->decoInstances.push_back(inst);
    }

    int startX = 1;
    for (size_t i = 0; i < title.size() && startX + static_cast<int>(i) < cols - 1; ++i) {
        char32_t cp = static_cast<unsigned char>(title[i]);
        const GlyphInfo* glyph = impl_->atlas->GetGlyph(cp);
        if (!glyph) glyph = impl_->atlas->GetGlyph(' ');
        if (!glyph) continue;

        CellInstance inst;
        inst.colRow[0] = static_cast<float>(startX + i);
        inst.colRow[1] = 0.0f;
        inst.color[0] = ColorR(fg) / 255.0f;
        inst.color[1] = ColorG(fg) / 255.0f;
        inst.color[2] = ColorB(fg) / 255.0f;
        inst.color[3] = ColorA(fg) / 255.0f;
        inst.bgColor[0] = ColorR(bg) / 255.0f;
        inst.bgColor[1] = ColorG(bg) / 255.0f;
        inst.bgColor[2] = ColorB(bg) / 255.0f;
        inst.bgColor[3] = ColorA(bg) / 255.0f;
        inst.uv[0] = glyph->u0;
        inst.uv[1] = glyph->v0;
        inst.uv[2] = glyph->u1;
        inst.uv[3] = glyph->v1;
        inst.glyphScale[0] = 1.0f;
        inst.glyphScale[1] = 1.0f;
        inst.glyphOffset[0] = 0.0f;
        inst.glyphOffset[1] = 0.0f;
        impl_->decoInstances.push_back(inst);
    }
}

void Renderer::DrawBorder(int cols, int rows, int titleBarRows, bool active) {
    if (!impl_->atlas) return;

    impl_->titleBarRows_ = titleBarRows;
    impl_->contentOffsetX_ = 1;
    impl_->contentOffsetY_ = titleBarRows + 1;

    Color fg = active ? Theme::BorderActive : Theme::BorderInactive;
    Color bg = Theme::DefaultBg;

    int left = 0;
    int right = cols - 1;
    int top = titleBarRows;
    int bottom = titleBarRows + rows - 1;

    if (top > bottom || left > right) return;

    auto addCell = [&](int x, int y, char32_t cp) {
        const GlyphInfo* glyph = impl_->atlas->GetGlyph(cp);
        if (!glyph) glyph = impl_->atlas->GetGlyph(' ');
        if (!glyph) return;

        CellInstance inst;
        inst.colRow[0] = static_cast<float>(x);
        inst.colRow[1] = static_cast<float>(y);
        inst.color[0] = ColorR(fg) / 255.0f;
        inst.color[1] = ColorG(fg) / 255.0f;
        inst.color[2] = ColorB(fg) / 255.0f;
        inst.color[3] = ColorA(fg) / 255.0f;
        inst.bgColor[0] = ColorR(bg) / 255.0f;
        inst.bgColor[1] = ColorG(bg) / 255.0f;
        inst.bgColor[2] = ColorB(bg) / 255.0f;
        inst.bgColor[3] = ColorA(bg) / 255.0f;
        inst.uv[0] = glyph->u0;
        inst.uv[1] = glyph->v0;
        inst.uv[2] = glyph->u1;
        inst.uv[3] = glyph->v1;
        inst.glyphScale[0] = 1.0f;
        inst.glyphScale[1] = 1.0f;
        inst.glyphOffset[0] = 0.0f;
        inst.glyphOffset[1] = 0.0f;
        impl_->decoInstances.push_back(inst);
    };

    addCell(left, top, U'\u250C');
    addCell(right, top, U'\u2510');
    addCell(left, bottom, U'\u2514');
    addCell(right, bottom, U'\u2518');

    for (int x = left + 1; x < right; ++x) {
        addCell(x, top, U'\u2500');
        addCell(x, bottom, U'\u2500');
    }
    for (int y = top + 1; y < bottom; ++y) {
        addCell(left, y, U'\u2502');
        addCell(right, y, U'\u2502');
    }
}

void Renderer::DrawScrollBar(int cols, int rows, int scrollPos, int contentHeight, int titleBarRows) {
    if (!impl_->atlas) return;
    int trackY = titleBarRows;
    int trackH = rows;
    int thumbSize = trackH;
    if (contentHeight > trackH) {
        thumbSize = std::max(1, trackH * trackH / contentHeight);
    }
    int thumbPos = 0;
    if (contentHeight > trackH) {
        thumbPos = scrollPos * (trackH - thumbSize) / (contentHeight - trackH);
    }

    int barX = cols - 1;
    Color bg = Theme::ScrollBarBg;
    Color thumb = Theme::ScrollBarThumb;

    for (int y = 0; y < trackH; ++y) {
        char32_t cp = U'\u2502'; // track
        Color cellColor = bg;
        if (y >= thumbPos && y < thumbPos + thumbSize) {
            cp = U'\u2588'; // thumb block
            cellColor = thumb;
        }

        const GlyphInfo* glyph = impl_->atlas->GetGlyph(cp);
        if (!glyph) glyph = impl_->atlas->GetGlyph(' ');
        if (!glyph) continue;

        CellInstance inst;
        inst.colRow[0] = static_cast<float>(barX);
        inst.colRow[1] = static_cast<float>(trackY + y);
        inst.color[0] = ColorR(cellColor) / 255.0f;
        inst.color[1] = ColorG(cellColor) / 255.0f;
        inst.color[2] = ColorB(cellColor) / 255.0f;
        inst.color[3] = ColorA(cellColor) / 255.0f;
        inst.bgColor[0] = ColorR(bg) / 255.0f;
        inst.bgColor[1] = ColorG(bg) / 255.0f;
        inst.bgColor[2] = ColorB(bg) / 255.0f;
        inst.bgColor[3] = ColorA(bg) / 255.0f;
        inst.uv[0] = glyph->u0;
        inst.uv[1] = glyph->v0;
        inst.uv[2] = glyph->u1;
        inst.uv[3] = glyph->v1;
        inst.glyphScale[0] = 1.0f;
        inst.glyphScale[1] = 1.0f;
        inst.glyphOffset[0] = 0.0f;
        inst.glyphOffset[1] = 0.0f;
        impl_->decoInstances.push_back(inst);
    }
}

void Renderer::SwapBuffers() {
    if (impl_->window) {
        SDL_GL_SwapWindow(static_cast<SDL_Window*>(impl_->window->GetNativeHandle()));
    }
}

void Renderer::SetViewport(int pixelWidth, int pixelHeight) {
    glViewport(0, 0, pixelWidth, pixelHeight);
}

} // namespace bui
