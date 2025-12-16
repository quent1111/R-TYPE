#pragma once

#include <SFML/Graphics.hpp>

#include <iostream>
#include <string>
#include <unordered_map>

namespace managers {

class TextureManager {
public:
    static TextureManager& instance();

    TextureManager(const TextureManager&) = delete;
    TextureManager& operator=(const TextureManager&) = delete;

    sf::Texture& load(const std::string& filepath);
    sf::Texture* get(const std::string& filepath);
    bool has(const std::string& filepath) const;
    void unload(const std::string& filepath);
    void clear();
    size_t size() const;

private:
    TextureManager() = default;
    ~TextureManager() = default;

    std::unordered_map<std::string, sf::Texture> textures_;
};

}  // namespace managers
