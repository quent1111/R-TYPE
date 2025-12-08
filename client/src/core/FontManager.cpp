#include "core/FontManager.hpp"

#include <iostream>

namespace core {

FontManager& FontManager::instance() {
    static FontManager instance;
    return instance;
}

const sf::Font* FontManager::load_font(const std::string& filepath) {
    auto it = fonts_.find(filepath);
    if (it != fonts_.end()) {
        return it->second.get();
    }
    auto font = std::make_unique<sf::Font>();
    if (!font->loadFromFile(filepath)) {
        std::cerr << "Failed to load font: " << filepath << std::endl;
        return nullptr;
    }
    const sf::Font* font_ptr = font.get();
    fonts_[filepath] = std::move(font);
    return font_ptr;
}

const sf::Font* FontManager::get_default_font() {
    return load_font(default_font_path_);
}

}  // namespace core
