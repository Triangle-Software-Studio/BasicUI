#pragma once

#include "basicui/basicui.h"
#include "basicui/render/glyph_atlas.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

namespace bui {

// Global font registry supporting runtime font / size changes.
class FontManager {
public:
    static FontManager& Instance();

    // Register a font file under a logical name, e.g. "default", "title", "mono"
    bool Register(const std::string& name, const std::string& path, int pixelHeight);

    // Query / load an atlas for a registered font.
    std::shared_ptr<GlyphAtlas> GetAtlas(const std::string& name);

    // Change the default font used by all widgets unless overridden.
    void SetDefaultFont(const std::string& name);
    const std::string& GetDefaultFont() const { return defaultFont_; }

    const FontDesc* GetDesc(const std::string& name) const;

private:
    FontManager() = default;

    struct Entry {
        FontDesc desc;
        std::weak_ptr<GlyphAtlas> atlas;
    };

    std::unordered_map<std::string, Entry> entries_;
    std::string defaultFont_ = "default";
};

} // namespace bui
