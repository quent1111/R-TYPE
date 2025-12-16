#pragma once

#include <SFML/Graphics.hpp>

#include <map>
#include <memory>
#include <string>

namespace managers {

class FontManager {
public:
    static FontManager& instance();

    FontManager(const FontManager&) = delete;
    FontManager& operator=(const FontManager&) = delete;

    const sf::Font* load(const std::string& filepath);
    const sf::Font* get(const std::string& filepath);
    const sf::Font* get_default();
    bool has(const std::string& filepath) const;

private:
    FontManager() = default;
    ~FontManager() = default;

    std::map<std::string, std::unique_ptr<sf::Font>> fonts_;
    static constexpr const char* DEFAULT_FONT_PATH = "assets/fonts/arial.ttf";
};

}  // namespace managers
