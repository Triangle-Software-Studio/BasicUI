#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include <memory>
#include "basicui/basicui.h"

struct FT_LibraryRec_;
struct FT_FaceRec_;

namespace bui {

struct GlyphInfo {
    float u0 = 0, v0 = 0; // top-left UV
    float u1 = 0, v1 = 0; // bottom-right UV
    int bearingX = 0;
    int bearingY = 0;
    int advance = 0;
    int width = 0;
    int height = 0;
};

// Font face configuration used by GlyphAtlas and Theme
struct FontDesc {
    std::string path;
    int pixelHeight = 16;

    bool operator==(const FontDesc& o) const {
        return path == o.path && pixelHeight == o.pixelHeight;
    }
};

class GlyphAtlas {
public:
    GlyphAtlas();
    ~GlyphAtlas();

    GlyphAtlas(const GlyphAtlas&) = delete;
    GlyphAtlas& operator=(const GlyphAtlas&) = delete;

    bool LoadFont(const std::string& fontPath, int pixelHeight);
    bool LoadFontFromMemory(const std::vector<uint8_t>& data, int pixelHeight);

    const GlyphInfo* GetGlyph(char32_t codepoint);
    int CellWidth() const { return cellWidth_; }
    int CellHeight() const { return cellHeight_; }
    int Ascent() const { return ascent_; }

    // OpenGL texture id
    unsigned int TextureId() const { return textureId_; }
    int TextureWidth() const { return textureWidth_; }
    int TextureHeight() const { return textureHeight_; }

    const FontDesc& Font() const { return font_; }

private:
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

    bool RasterizeGlyph(char32_t codepoint, GlyphInfo& out);
    void PackTexture();
    void Reset();

    FT_LibraryRec_* ftLibrary_ = nullptr;
    FT_FaceRec_* ftFace_ = nullptr;
    FontDesc font_;
    int pixelHeight_ = 0;
    int cellWidth_ = 0;
    int cellHeight_ = 0;
    int ascent_ = 0;

    unsigned int textureId_ = 0;
    int textureWidth_ = 0;
    int textureHeight_ = 0;

    std::unordered_map<char32_t, GlyphInfo> glyphs_;
    std::vector<char32_t> pending_;
    std::unordered_map<char32_t, StagedGlyph> staging_;
};

} // namespace bui
