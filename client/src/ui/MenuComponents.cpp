#include "ui/MenuComponents.hpp"

#include <cmath>
#include <iostream>

namespace rtype::ui {


Button::Button(const sf::Vector2f& position, const sf::Vector2f& size,
               const std::string& text) {
    m_shape.setPosition(position);
    m_shape.setSize(size);
    m_shape.setFillColor(m_normal_color);
    m_shape.setOutlineThickness(2.0f);
    m_shape.setOutlineColor(sf::Color(150, 150, 150));

    if (!m_font.loadFromFile("assets/fonts/arial.ttf")) {
        std::cerr << "Warning: Could not load font for button\n";
    }

    m_text.setFont(m_font);
    m_text.setString(text);
    m_text.setCharacterSize(24);
    m_text.setFillColor(sf::Color::White);

    sf::FloatRect text_bounds = m_text.getLocalBounds();
    m_text.setOrigin(text_bounds.left + text_bounds.width / 2.0f,
                     text_bounds.top + text_bounds.height / 2.0f);
    m_text.setPosition(position.x + size.x / 2.0f, position.y + size.y / 2.0f);
}

void Button::handle_mouse_move(const sf::Vector2f& mouse_pos) {
    m_is_hovered = contains(mouse_pos);
}

bool Button::handle_mouse_click(const sf::Vector2f& mouse_pos) {
    if (contains(mouse_pos)) {
        m_is_pressed = true;
        if (m_callback) {
            m_callback();
        }
        return true;
    }
    return false;
}

void Button::update(float dt) {
    (void)dt;

    if (m_is_pressed) {
        m_shape.setFillColor(m_pressed_color);
        m_is_pressed = false;
    } else if (m_is_hovered) {
        m_shape.setFillColor(m_hover_color);
    } else {
        m_shape.setFillColor(m_normal_color);
    }
}

void Button::render(sf::RenderWindow& window) {
    window.draw(m_shape);
    window.draw(m_text);
}

bool Button::contains(const sf::Vector2f& point) const {
    return m_shape.getGlobalBounds().contains(point);
}

void Button::set_text(const std::string& text) {
    m_text.setString(text);
    
    sf::FloatRect text_bounds = m_text.getLocalBounds();
    m_text.setOrigin(text_bounds.left + text_bounds.width / 2.0f,
                     text_bounds.top + text_bounds.height / 2.0f);
}

void Button::set_colors(const sf::Color& normal, const sf::Color& hover,
                        const sf::Color& pressed) {
    m_normal_color = normal;
    m_hover_color = hover;
    m_pressed_color = pressed;
}


MenuTitle::MenuTitle(const std::string& text, const sf::Vector2f& position,
                     unsigned int size) {
    if (!m_font.loadFromFile("assets/fonts/arial.ttf")) {
        std::cerr << "Warning: Could not load font for title\n";
    }

    m_text.setFont(m_font);
    m_text.setString(text);
    m_text.setCharacterSize(size);
    m_text.setFillColor(sf::Color::White);
    m_text.setStyle(sf::Text::Bold);

    sf::FloatRect bounds = m_text.getLocalBounds();
    m_text.setOrigin(bounds.left + bounds.width / 2.0f,
                     bounds.top + bounds.height / 2.0f);
    m_text.setPosition(position);
}

void MenuTitle::render(sf::RenderWindow& window) { window.draw(m_text); }

void MenuTitle::set_text(const std::string& text) { m_text.setString(text); }


MenuBackground::MenuBackground(const sf::Vector2u& window_size) {
    m_background.setSize(sf::Vector2f(static_cast<float>(window_size.x),
                                      static_cast<float>(window_size.y)));
    m_background.setFillColor(sf::Color(20, 20, 30));

    m_overlay.setSize(sf::Vector2f(static_cast<float>(window_size.x),
                                   static_cast<float>(window_size.y)));
    m_overlay.setFillColor(sf::Color(0, 0, 0, 50));
}

void MenuBackground::update(float dt) {
    m_pulse_time += dt;
    
    float alpha = 50.0f + 20.0f * std::sin(m_pulse_time * 2.0f);
    m_overlay.setFillColor(sf::Color(0, 0, 0, static_cast<sf::Uint8>(alpha)));
}

void MenuBackground::render(sf::RenderWindow& window) {
    window.draw(m_background);
    window.draw(m_overlay);
}

} // namespace rtype::ui
