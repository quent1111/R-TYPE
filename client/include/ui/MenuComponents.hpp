#pragma once

#include <SFML/Graphics.hpp>

#include <functional>
#include <string>
#include <vector>

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
    void trigger() { if (m_callback) m_callback(); }
    void set_hovered(bool hovered) { m_is_hovered = hovered; }

private:
    sf::ConvexShape m_shape;
    sf::ConvexShape m_inner_shape;
    std::vector<sf::ConvexShape> m_glow_layers;
    std::vector<sf::RectangleShape> m_side_lines;
    std::vector<sf::CircleShape> m_corner_dots;
    sf::RectangleShape m_scan_line;
    sf::Text m_text;
    std::function<void()> m_callback;
    sf::Vector2f m_position;
    sf::Vector2f m_size;
    sf::Color m_normal_color{70, 70, 70};
    sf::Color m_hover_color{100, 100, 100};
    sf::Color m_pressed_color{50, 50, 50};
    bool m_is_hovered{false};
    bool m_is_pressed{false};
    float m_hover_time{0.0f};
    float m_scan_time{0.0f};
};

class MenuTitle {
public:
    MenuTitle(const std::string& text, const sf::Vector2f& position, unsigned int size = 72);

    void update(float dt);
    void render(sf::RenderWindow& window);
    void set_text(const std::string& text);
    bool has_logo() const { return m_has_logo; }

private:
    sf::Text m_text;
    sf::Text m_shadow;
    sf::Texture m_logo_texture;
    sf::Sprite m_logo_sprite;
    sf::RectangleShape m_logo_glow;
    bool m_has_logo{false};
    float m_glow_time{0.0f};
    std::vector<sf::CircleShape> m_particles;
};

class MenuBackground {
public:
    MenuBackground(const sf::Vector2u& window_size);

    void update(float dt);
    void render(sf::RenderWindow& window);

private:
    struct Star {
        sf::Vector2f position;
        float speed;
        float size;
        sf::Color color;
    };

    struct AnimatedShip {
        sf::Sprite sprite;
        std::vector<sf::IntRect> frames;
        size_t current_frame;
        float frame_duration;
        float time_accumulator;
        sf::Vector2f position;
        sf::Vector2f velocity;
        float scale;
        float rotation_speed;
    };

    struct Asteroid {
        sf::Sprite sprite;
        sf::Vector2f position;
        sf::Vector2f velocity;
        float rotation;
        float rotation_speed;
        float scale;
    };

    sf::RectangleShape m_background;
    sf::Texture m_background_texture;
    sf::Sprite m_background_sprite;
    std::vector<Star> m_stars;
    std::vector<AnimatedShip> m_ships;
    std::vector<sf::Texture> m_ship_textures;
    std::vector<Asteroid> m_asteroids;
    std::vector<sf::Texture> m_asteroid_textures;
    sf::Vector2u m_window_size;
    float m_time{0.0f};
};

class ParticleEffect {
public:
    ParticleEffect(const sf::Vector2f& position);

    void update(float dt);
    void render(sf::RenderWindow& window);
    bool is_alive() const;

private:
    struct Particle {
        sf::Vector2f position;
        sf::Vector2f velocity;
        float lifetime;
        float max_lifetime;
        sf::Color color;
    };

    std::vector<Particle> m_particles;
};

class MenuFooter {
public:
    MenuFooter(const sf::Vector2u& window_size);
    void render(sf::RenderWindow& window);

private:
    sf::Text m_version_text;
    sf::Text m_copyright_text;
    std::vector<sf::RectangleShape> m_decorative_lines;
};

class CornerDecoration {
public:
    CornerDecoration(const sf::Vector2f& position, bool flip_x, bool flip_y);
    void update(float dt);
    void render(sf::RenderWindow& window);

private:
    std::vector<sf::RectangleShape> m_lines;
    std::vector<sf::CircleShape> m_dots;
    float m_glow_time{0.0f};
};

class SidePanel {
public:
    SidePanel(const sf::Vector2f& position, bool is_left);
    void update(float dt);
    void render(sf::RenderWindow& window);

private:
    std::vector<sf::RectangleShape> m_bars;
    std::vector<sf::CircleShape> m_indicators;
    sf::Text m_label;
    float m_anim_time{0.0f};
};

}  // namespace rtype::ui
