#include "managers/FontManager.hpp"

#include <iostream>

namespace managers {

FontManager& FontManager::instance() {
    static FontManager instance;
    return instance;
}

const sf::Font* FontManager::load(const std::string& filepath) {
    auto it = fonts_.find(filepath);
    if (it != fonts_.end()) {
        return it->second.get();
    }

    auto font = std::make_unique<sf::Font>();
    if (!font->loadFromFile(filepath)) {
        std::cerr << "ERROR: Failed to load font: " << filepath << std::endl;
        return nullptr;
    }

    std::cout << "Loaded font: " << filepath << std::endl;
    fonts_[filepath] = std::move(font);
    return fonts_[filepath].get();
}

const sf::Font* FontManager::get(const std::string& filepath) {
    auto it = fonts_.find(filepath);
    return (it != fonts_.end()) ? it->second.get() : nullptr;
}

const sf::Font* FontManager::get_default() {
    return load(DEFAULT_FONT_PATH);
}

bool FontManager::has(const std::string& filepath) const {
    return fonts_.find(filepath) != fonts_.end();
}

}  // namespace managers
