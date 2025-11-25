#pragma once

#include <SFML/Graphics.hpp>
#include <functional>
#include <string>

namespace rtype::ui {

class Button {
public:
    Button(const sf::Vector2f& position, const sf::Vector2f& size, const std::string& text);

    void set_callback(std::function<void()> callback) { m_callback = std::move(callback); }
    void handle_mouse_move(const sf::Vector2f& mouse_pos);
    bool handle_mouse_click(const sf::Vector2f& mouse_pos);
    void update(float dt);
    void render(sf::RenderWindow& window);
    bool contains(const sf::Vector2f& point) const;
    void set_text(const std::string& text);
    void set_colors(const sf::Color& normal, const sf::Color& hover, const sf::Color& pressed);

private:
    sf::RectangleShape m_shape;
    sf::Text m_text;
    sf::Font m_font;
    std::function<void()> m_callback;
    sf::Color m_normal_color{70, 70, 70};
    sf::Color m_hover_color{100, 100, 100};
    sf::Color m_pressed_color{50, 50, 50};
    bool m_is_hovered{false};
    bool m_is_pressed{false};
};

class MenuTitle {
public:
    MenuTitle(const std::string& text, const sf::Vector2f& position, unsigned int size = 72);

    void render(sf::RenderWindow& window);
    void set_text(const std::string& text);

private:
    sf::Text m_text;
    sf::Font m_font;
};

class MenuBackground {
public:
    MenuBackground(const sf::Vector2u& window_size);

    void update(float dt);
    void render(sf::RenderWindow& window);

private:
    sf::RectangleShape m_background;
    sf::RectangleShape m_overlay;
    float m_pulse_time{0.0f};
};

} // namespace rtype::ui
