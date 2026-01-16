#pragma once

#include <SFML/Graphics.hpp>
#include <ColorBlindnessMode.hpp>

namespace rendering {

class ColorBlindShader {
public:
    static ColorBlindShader& instance();
    
    void init();
    void apply(sf::RenderWindow& window);
    bool is_loaded() const { return shader_loaded_; }
    
private:
    ColorBlindShader() = default;
    
    sf::Shader shader_;
    bool shader_loaded_ = false;
    bool initialized_ = false;
};

}  // namespace rendering
