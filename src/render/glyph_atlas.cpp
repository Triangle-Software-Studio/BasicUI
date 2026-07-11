#include "basicui/render/glyph_atlas.h"
#include "basicui/basicui.h"
#include <GL/glew.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <vector>
#include <algorithm>
#include <fstream>

namespace bui {

namespace {
    struct Bitmap {
        int width = 0;
        int height = 0;
        std::vector<uint8_t> data;
    };

    struct StagedGlyph {
        Bitmap bitmap;
        int bearingX = 0;
        int bearingY = 0;
        int advance = 0;
    };

    std::unordered_map<const GlyphAtlas*, std::unordered_map<char32_t, StagedGlyph>> g_staging;
}

GlyphAtlas::GlyphAtlas() {}

GlyphAtlas::~GlyphAtlas() {
    Reset();
}

void GlyphAtlas::Reset() {
    if (textureId_) {
        glDeleteTextures(1, &textureId_);
        textureId_ = 0;
    }
    if (ftFace_) {
        FT_Done_Face(reinterpret_cast<FT_Face>(ftFace_));
        ftFace_ = nullptr;
    }
    if (ftLibrary_) {
        FT_Done_FreeType(reinterpret_cast<FT_Library>(ftLibrary_));
        ftLibrary_ = nullptr;
    }
    g_staging.erase(this);
    glyphs_.clear();
    pending_.clear();
    pixelHeight_ = 0;
    cellWidth_ = 0;
    cellHeight_ = 0;
    ascent_ = 0;
    font_ = {};
}

bool GlyphAtlas::LoadFont(const std::string& fontPath, int pixelHeight) {
    Reset();

    FT_Library lib = nullptr;
    if (FT_Init_FreeType(&lib) != 0) return false;
    ftLibrary_ = reinterpret_cast<FT_LibraryRec_*>(lib);

    FT_Face face = nullptr;
    if (FT_New_Face(lib, fontPath.c_str(), 0, &face) != 0) {
        FT_Done_FreeType(lib);
        ftLibrary_ = nullptr;
        return false;
    }
    ftFace_ = reinterpret_cast<FT_FaceRec_*>(face);

    font_.path = fontPath;
    font_.pixelHeight = pixelHeight;
    pixelHeight_ = pixelHeight;
    FT_Set_Pixel_Sizes(face, 0, pixelHeight_);

    cellWidth_ = static_cast<int>(face->size->metrics.max_advance / 64);
    if (cellWidth_ <= 0) cellWidth_ = pixelHeight_ / 2;
    cellHeight_ = static_cast<int>(face->size->metrics.height / 64);
    if (cellHeight_ <= 0) cellHeight_ = pixelHeight_;
    ascent_ = static_cast<int>(face->size->metrics.ascender / 64);
    if (ascent_ <= 0) ascent_ = cellHeight_;

    // Default codepoint set (ASCII + common box-drawing + block characters)
    static const std::vector<char32_t> kDefaultCodepoints = [] {
        std::vector<char32_t> cp;
        for (char32_t c = 32; c <= 126; ++c) cp.push_back(c);
        cp.insert(cp.end(), {0x2500, 0x2502, 0x250C, 0x2510, 0x2514, 0x2518, 0x2588,
                             0x2591, 0x2592, 0x2593, 0x2550, 0x2551, 0x2554, 0x2557,
                             0x255A, 0x255D});
        return cp;
    }();

    for (char32_t c : kDefaultCodepoints) {
        GlyphInfo info;
        RasterizeGlyph(c, info);
    }

    PackTexture();
    g_staging.erase(this);
    return textureId_ != 0;
}

bool GlyphAtlas::LoadFontFromMemory(const std::vector<uint8_t>& data, int pixelHeight) {
    Reset();

    FT_Library lib = nullptr;
    if (FT_Init_FreeType(&lib) != 0) return false;
    ftLibrary_ = reinterpret_cast<FT_LibraryRec_*>(lib);

    FT_Face face = nullptr;
    if (FT_New_Memory_Face(lib, data.data(), static_cast<FT_Long>(data.size()), 0, &face) != 0) {
        FT_Done_FreeType(lib);
        ftLibrary_ = nullptr;
        return false;
    }
    ftFace_ = reinterpret_cast<FT_FaceRec_*>(face);

    font_.path = "<memory>";
    font_.pixelHeight = pixelHeight;
    pixelHeight_ = pixelHeight;
    FT_Set_Pixel_Sizes(face, 0, pixelHeight_);

    cellWidth_ = static_cast<int>(face->size->metrics.max_advance / 64);
    if (cellWidth_ <= 0) cellWidth_ = pixelHeight_ / 2;
    cellHeight_ = static_cast<int>(face->size->metrics.height / 64);
    if (cellHeight_ <= 0) cellHeight_ = pixelHeight_;
    ascent_ = static_cast<int>(face->size->metrics.ascender / 64);
    if (ascent_ <= 0) ascent_ = cellHeight_;

    // Default codepoint set (ASCII + common box-drawing + block characters)
    static const std::vector<char32_t> kDefaultCodepoints = [] {
        std::vector<char32_t> cp;
        for (char32_t c = 32; c <= 126; ++c) cp.push_back(c);
        cp.insert(cp.end(), {0x2500, 0x2502, 0x250C, 0x2510, 0x2514, 0x2518, 0x2588,
                             0x2591, 0x2592, 0x2593, 0x2550, 0x2551, 0x2554, 0x2557,
                             0x255A, 0x255D});
        return cp;
    }();

    for (char32_t c : kDefaultCodepoints) {
        GlyphInfo info;
        RasterizeGlyph(c, info);
    }

    PackTexture();
    g_staging.erase(this);
    return textureId_ != 0;
}

const GlyphInfo* GlyphAtlas::GetGlyph(char32_t codepoint) const {
    auto it = glyphs_.find(codepoint);
    if (it != glyphs_.end()) return &it->second;
    return nullptr;
}

bool GlyphAtlas::RasterizeGlyph(char32_t codepoint, GlyphInfo& out) {
    if (!ftFace_) return false;
    FT_Face face = reinterpret_cast<FT_Face>(ftFace_);
    if (FT_Load_Char(face, codepoint, FT_LOAD_RENDER | FT_LOAD_TARGET_NORMAL) != 0) return false;

    FT_GlyphSlot slot = face->glyph;
    Bitmap bmp;
    bmp.width = slot->bitmap.width;
    bmp.height = slot->bitmap.rows;

    if (bmp.width > 0 && bmp.height > 0) {
        bmp.data.resize(bmp.width * bmp.height);
        if (slot->bitmap.pixel_mode == FT_PIXEL_MODE_MONO) {
            // 1-bit packed bitmap: unpack each bit into a byte.
            for (int row = 0; row < bmp.height; ++row) {
                const uint8_t* src = slot->bitmap.buffer + row * slot->bitmap.pitch;
                for (int col = 0; col < bmp.width; ++col) {
                    int byteIndex = col >> 3;
                    int bitIndex = 7 - (col & 7);
                    uint8_t value = (src[byteIndex] >> bitIndex) & 1;
                    bmp.data[row * bmp.width + col] = value ? 255 : 0;
                }
            }
        } else {
            for (int row = 0; row < bmp.height; ++row) {
                for (int col = 0; col < bmp.width; ++col) {
                    bmp.data[row * bmp.width + col] = slot->bitmap.buffer[row * slot->bitmap.pitch + col];
                }
            }
        }
    }

    out.width = bmp.width;
    out.height = bmp.height;
    out.bearingX = slot->bitmap_left;
    out.bearingY = slot->bitmap_top;
    out.advance = static_cast<int>(slot->advance.x >> 6);

    StagedGlyph staged;
    staged.bitmap = std::move(bmp);
    staged.bearingX = out.bearingX;
    staged.bearingY = out.bearingY;
    staged.advance = out.advance;
    g_staging[this][codepoint] = std::move(staged);
    return true;
}

void GlyphAtlas::PackTexture() {
    auto it = g_staging.find(this);
    if (it == g_staging.end() || it->second.empty()) return;

    int texSize = 512;
    auto& staging = it->second;

    // Sort glyphs by height descending for better packing density.
    std::vector<std::pair<char32_t, StagedGlyph*>> sorted;
    sorted.reserve(staging.size());
    for (auto& kv : staging) {
        sorted.push_back({kv.first, &kv.second});
    }
    std::sort(sorted.begin(), sorted.end(), [](const auto& a, const auto& b) {
        return a.second->bitmap.height > b.second->bitmap.height;
    });

    for (int attempt = 0; attempt < 2; ++attempt) {
        std::vector<uint8_t> buffer(texSize * texSize, 0);
        int x = 0;
        int y = 0;
        int rowHeight = 0;
        bool ok = true;

        for (auto& kv : sorted) {
            char32_t cp = kv.first;
            StagedGlyph& staged = *kv.second;

            // Embed the glyph bitmap into a cell-sized canvas so the renderer
            // can draw each cell with glyphScale=(1,1) without stretching.
            int canvasW = cellWidth_;
            int canvasH = cellHeight_;
            int w = canvasW + 1; // 1px padding
            int h = canvasH + 1;

            if (x + w > texSize) {
                x = 0;
                y += rowHeight;
                rowHeight = 0;
            }
            if (y + h > texSize) {
                ok = false;
                break;
            }
            rowHeight = std::max(rowHeight, h);

            // Calculate bitmap position inside the cell canvas
            int dstX = staged.bearingX;
            int dstY = ascent_ - staged.bearingY;

            // Copy bitmap into canvas with clipping
            for (int row = 0; row < staged.bitmap.height; ++row) {
                int canvasRow = dstY + row;
                if (canvasRow < 0 || canvasRow >= canvasH) continue;
                for (int col = 0; col < staged.bitmap.width; ++col) {
                    int canvasCol = dstX + col;
                    if (canvasCol < 0 || canvasCol >= canvasW) continue;
                    buffer[(y + canvasRow) * texSize + (x + canvasCol)] =
                        staged.bitmap.data[row * staged.bitmap.width + col];
                }
            }

            GlyphInfo info;
            info.width = canvasW;
            info.height = canvasH;
            info.bearingX = staged.bearingX;
            info.bearingY = staged.bearingY;
            info.advance = staged.advance;
            float halfU = 0.5f / texSize;
            float halfV = 0.5f / texSize;
            info.u0 = static_cast<float>(x) / texSize + halfU;
            info.u1 = static_cast<float>(x + canvasW) / texSize - halfU;
            // v0 maps to the visual top of the glyph (smaller texture V),
            // v1 maps to the visual bottom (larger texture V).
            info.v0 = static_cast<float>(y) / texSize + halfV;
            info.v1 = static_cast<float>(y + canvasH) / texSize - halfV;

            glyphs_[cp] = info;
            x += w;
        }

        if (ok) {
            if (textureId_) {
                glDeleteTextures(1, &textureId_);
            }
            glGenTextures(1, &textureId_);
            glBindTexture(GL_TEXTURE_2D, textureId_);
            glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, texSize, texSize, 0, GL_RED, GL_UNSIGNED_BYTE, buffer.data());
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

            textureWidth_ = texSize;
            textureHeight_ = texSize;
            return;
        }

        texSize = 1024;
    }
}

} // namespace bui
