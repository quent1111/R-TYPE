#pragma once

#include <SFML/Graphics.hpp>

#include <iostream>
#include <string>
#include <unordered_map>

class TextureManager {
public:
    sf::Texture& load(const std::string& filepath) {
        auto it = _textures.find(filepath);
        if (it != _textures.end()) {
            return it->second;
        }

        sf::Texture texture;
        if (!texture.loadFromFile(filepath)) {
            std::cerr << "ERROR: Failed to load texture: " << filepath << std::endl;
            throw std::runtime_error("Failed to load texture: " + filepath);
        }

        std::cout << "Loaded texture: " << filepath << std::endl;

        _textures[filepath] = std::move(texture);
        return _textures[filepath];
    }

    sf::Texture* get(const std::string& filepath) {
        auto it = _textures.find(filepath);
        return (it != _textures.end()) ? &it->second : nullptr;
    }

    bool has(const std::string& filepath) const {
        return _textures.find(filepath) != _textures.end();
    }

    void unload(const std::string& filepath) { _textures.erase(filepath); }

    void clear() {
        _textures.clear();
        std::cout << "All textures cleared from cache" << std::endl;
    }

    size_t size() const { return _textures.size(); }

private:
    std::unordered_map<std::string, sf::Texture> _textures;
};
