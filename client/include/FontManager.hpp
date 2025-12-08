#pragma once

#include <SFML/Graphics.hpp>

#include <map>
#include <memory>
#include <string>

namespace core {

class FontManager {
public:
    static FontManager& instance();
    const sf::Font* load_font(const std::string& filepath);
    const sf::Font* get_default_font();
    FontManager(const FontManager&) = delete;
    FontManager& operator=(const FontManager&) = delete;

private:
    FontManager() = default;
    std::map<std::string, std::unique_ptr<sf::Font>> fonts_;
    const std::string default_font_path_ = "assets/fonts/arial.ttf";
};

}  // namespace core
