#include "managers/TextureManager.hpp"

namespace managers {

TextureManager& TextureManager::instance() {
    static TextureManager instance;
    return instance;
}

sf::Texture& TextureManager::load(const std::string& filepath) {
    auto it = textures_.find(filepath);
    if (it != textures_.end()) {
        return it->second;
    }
    sf::Texture texture;
    if (!texture.loadFromFile(filepath)) {
        std::cerr << "ERROR: Failed to load texture: " << filepath << std::endl;
        throw std::runtime_error("Failed to load texture: " + filepath);
    }
    std::cout << "Loaded texture: " << filepath << std::endl;
    textures_[filepath] = std::move(texture);
    return textures_[filepath];
}

sf::Texture* TextureManager::get(const std::string& filepath) {
    auto it = textures_.find(filepath);
    return (it != textures_.end()) ? &it->second : nullptr;
}

bool TextureManager::has(const std::string& filepath) const {
    return textures_.find(filepath) != textures_.end();
}

void TextureManager::unload(const std::string& filepath) {
    textures_.erase(filepath);
}

void TextureManager::clear() {
    textures_.clear();
    std::cout << "All textures cleared from cache" << std::endl;
}

size_t TextureManager::size() const {
    return textures_.size();
}

}  // namespace managers
