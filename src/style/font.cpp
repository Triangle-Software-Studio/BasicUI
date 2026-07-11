#include "basicui/style/font.h"

namespace bui {

FontManager& FontManager::Instance() {
    static FontManager inst;
    return inst;
}

bool FontManager::Register(const std::string& name, const std::string& path, int pixelHeight) {
    entries_[name] = {{path, pixelHeight}, {}};
    return true;
}

std::shared_ptr<GlyphAtlas> FontManager::GetAtlas(const std::string& name) {
    auto it = entries_.find(name);
    if (it == entries_.end()) return nullptr;

    if (auto sp = it->second.atlas.lock()) {
        return sp;
    }

    auto atlas = std::make_shared<GlyphAtlas>();
    if (!atlas->LoadFont(it->second.desc.path, it->second.desc.pixelHeight)) {
        return nullptr;
    }
    it->second.atlas = atlas;
    return atlas;
}

void FontManager::SetDefaultFont(const std::string& name) {
    defaultFont_ = name;
}

const FontDesc* FontManager::GetDesc(const std::string& name) const {
    auto it = entries_.find(name);
    if (it == entries_.end()) return nullptr;
    return &it->second.desc;
}

} // namespace bui
