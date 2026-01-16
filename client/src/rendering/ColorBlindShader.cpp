#include "rendering/ColorBlindShader.hpp"
#include "common/Settings.hpp"
#include <iostream>

namespace rendering {

ColorBlindShader& ColorBlindShader::instance() {
    static ColorBlindShader instance;
    return instance;
}

void ColorBlindShader::init() {
    if (initialized_) {
        return;
    }

    initialized_ = true;

    if (!sf::Shader::isAvailable()) {
        std::cerr << "[ColorBlindShader] Shaders not available on this system" << std::endl;
        shader_loaded_ = false;
        return;
    }

    if (shader_.loadFromFile("assets/shaders/colorblind.vert", "assets/shaders/colorblind.frag")) {
        shader_loaded_ = true;
        std::cout << "[ColorBlindShader] Colorblind shader loaded successfully" << std::endl;
    } else {
        std::cerr << "[ColorBlindShader] Failed to load colorblind shader files" << std::endl;
        shader_loaded_ = false;
    }
}

void ColorBlindShader::apply(sf::RenderWindow& window) {
    if (!shader_loaded_) {
        return;
    }
    
    ColorBlindMode mode = Settings::instance().colorblind_mode;
    if (mode == ColorBlindMode::Normal) {
        return;
    }
    
    sf::Image screen_image = window.capture();
    sf::Texture screen_texture;
    screen_texture.loadFromImage(screen_image);

    sf::Sprite screen_sprite(screen_texture);
    shader_.setUniform("texture", sf::Shader::CurrentTexture);
    shader_.setUniform("mode", static_cast<int>(mode));

    window.clear();
    window.setView(window.getDefaultView());
    window.draw(screen_sprite, &shader_);
}

}  // namespace rendering
