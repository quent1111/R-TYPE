#pragma once

#include <SFML/Graphics.hpp>

#include <iostream>
#include <string>
#include <unordered_map>

/**
 * @brief Manages texture loading and caching for SFML
 *
 * TextureManager ensures textures are loaded only once and reused,
 * preventing duplicate loading and memory waste.
 */
class TextureManager {
public:
    /**
     * @brief Load a texture from file, or return cached version if already loaded
     * @param filepath Path to the texture file (relative to executable)
     * @return Reference to the loaded texture
     * @throws std::runtime_error if texture fails to load
     */
    sf::Texture& load(const std::string& filepath) {
        // Check if already loaded
        auto it = _textures.find(filepath);
        if (it != _textures.end()) {
            return it->second;
        }

        // Load new texture
        sf::Texture texture;
        if (!texture.loadFromFile(filepath)) {
            std::cerr << "ERROR: Failed to load texture: " << filepath << std::endl;
            throw std::runtime_error("Failed to load texture: " + filepath);
        }

        std::cout << "Loaded texture: " << filepath << std::endl;

        // Store and return
        _textures[filepath] = std::move(texture);
        return _textures[filepath];
    }

    /**
     * @brief Get a texture if it exists in cache
     * @param filepath Path to the texture
     * @return Pointer to texture if found, nullptr otherwise
     */
    sf::Texture* get(const std::string& filepath) {
        auto it = _textures.find(filepath);
        return (it != _textures.end()) ? &it->second : nullptr;
    }

    /**
     * @brief Check if a texture is already loaded
     */
    bool has(const std::string& filepath) const {
        return _textures.find(filepath) != _textures.end();
    }

    /**
     * @brief Remove a texture from cache
     */
    void unload(const std::string& filepath) { _textures.erase(filepath); }

    /**
     * @brief Clear all cached textures
     */
    void clear() {
        _textures.clear();
        std::cout << "All textures cleared from cache" << std::endl;
    }

    /**
     * @brief Get number of loaded textures
     */
    size_t size() const { return _textures.size(); }

private:
    std::unordered_map<std::string, sf::Texture> _textures;
};
