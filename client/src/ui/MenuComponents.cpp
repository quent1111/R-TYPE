#include "ui/MenuComponents.hpp"

#include <cmath>
#include <iostream>
#include <random>

namespace rtype::ui {

Button::Button(const sf::Vector2f& position, const sf::Vector2f& size,
               const std::string& text) : m_position(position), m_size(size) {
    const float bevel = 15.0f;
    m_shape.setPointCount(6);
    m_shape.setPoint(0, sf::Vector2f(bevel, 0));
    m_shape.setPoint(1, sf::Vector2f(size.x - bevel, 0));
    m_shape.setPoint(2, sf::Vector2f(size.x, size.y / 2.0f));
    m_shape.setPoint(3, sf::Vector2f(size.x - bevel, size.y));
    m_shape.setPoint(4, sf::Vector2f(bevel, size.y));
    m_shape.setPoint(5, sf::Vector2f(0, size.y / 2.0f));
    m_shape.setPosition(position);
    m_shape.setFillColor(m_normal_color);
    m_shape.setOutlineThickness(2.0f);
    m_shape.setOutlineColor(sf::Color(100, 200, 255, 200));

    m_inner_shape.setPointCount(6);
    const float inner_offset = 4.0f;
    m_inner_shape.setPoint(0, sf::Vector2f(bevel + inner_offset, inner_offset));
    m_inner_shape.setPoint(1, sf::Vector2f(size.x - bevel - inner_offset, inner_offset));
    m_inner_shape.setPoint(2, sf::Vector2f(size.x - inner_offset, size.y / 2.0f));
    m_inner_shape.setPoint(3, sf::Vector2f(size.x - bevel - inner_offset, size.y - inner_offset));
    m_inner_shape.setPoint(4, sf::Vector2f(bevel + inner_offset, size.y - inner_offset));
    m_inner_shape.setPoint(5, sf::Vector2f(inner_offset, size.y / 2.0f));
    m_inner_shape.setPosition(position);
    m_inner_shape.setFillColor(sf::Color::Transparent);
    m_inner_shape.setOutlineThickness(1.0f);
    m_inner_shape.setOutlineColor(sf::Color(100, 200, 255, 100));

    for (int i = 0; i < 3; ++i) {
        sf::ConvexShape glow;
        glow.setPointCount(6);
        float offset = 8.0f + static_cast<float>(i) * 4.0f;
        glow.setPoint(0, sf::Vector2f(bevel, 0));
        glow.setPoint(1, sf::Vector2f(size.x - bevel, 0));
        glow.setPoint(2, sf::Vector2f(size.x + offset, size.y / 2.0f));
        glow.setPoint(3, sf::Vector2f(size.x - bevel, size.y));
        glow.setPoint(4, sf::Vector2f(bevel, size.y));
        glow.setPoint(5, sf::Vector2f(-offset, size.y / 2.0f));
        glow.setPosition(position);
        glow.setFillColor(sf::Color::Transparent);
        glow.setOutlineThickness(2.0f);
        glow.setOutlineColor(sf::Color(100, 200, 255, 0));
        m_glow_layers.push_back(glow);
    }

    for (int i = 0; i < 4; ++i) {
        sf::CircleShape dot(3.0f);
        dot.setFillColor(sf::Color(100, 200, 255, 150));
        m_corner_dots.push_back(dot);
    }
    m_corner_dots[0].setPosition(position + sf::Vector2f(bevel - 3.0f, -3.0f));
    m_corner_dots[1].setPosition(position + sf::Vector2f(size.x - bevel - 3.0f, -3.0f));
    m_corner_dots[2].setPosition(position + sf::Vector2f(bevel - 3.0f, size.y - 3.0f));
    m_corner_dots[3].setPosition(position + sf::Vector2f(size.x - bevel - 3.0f, size.y - 3.0f));

    sf::RectangleShape line1(sf::Vector2f(30.0f, 1.5f));
    line1.setFillColor(sf::Color(100, 200, 255, 180));
    line1.setPosition(position + sf::Vector2f(10.0f, size.y / 2.0f));
    m_side_lines.push_back(line1);

    sf::RectangleShape line2(sf::Vector2f(30.0f, 1.5f));
    line2.setFillColor(sf::Color(100, 200, 255, 180));
    line2.setPosition(position + sf::Vector2f(size.x - 40.0f, size.y / 2.0f));
    m_side_lines.push_back(line2);

    m_scan_line.setSize(sf::Vector2f(size.x - 30.0f, 2.0f));
    m_scan_line.setFillColor(sf::Color(100, 220, 255, 0));
    m_scan_line.setPosition(position + sf::Vector2f(15.0f, 0));

    if (!m_font.loadFromFile("assets/fonts/arial.ttf")) {
        std::cerr << "Warning: Could not load font for button\n";
    }

    m_text.setFont(m_font);
    m_text.setString(text);
    m_text.setCharacterSize(24);
    m_text.setFillColor(sf::Color(220, 220, 220));
    m_text.setStyle(sf::Text::Bold);
    m_text.setLetterSpacing(1.5f);

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
    if (m_is_hovered) {
        m_hover_time += dt;
        m_scan_time += dt * 3.0f;
    } else {
        m_hover_time = 0.0f;
        m_scan_time = 0.0f;
    }

    if (m_is_pressed) {
        m_shape.setFillColor(m_pressed_color);
        m_shape.setOutlineColor(sf::Color(255, 180, 50, 255));
        m_inner_shape.setOutlineColor(sf::Color(255, 200, 100, 200));
        m_text.setFillColor(sf::Color(255, 255, 255, 255));
        for (auto& line : m_side_lines) {
            line.setFillColor(sf::Color(255, 200, 100, 255));
        }
        m_is_pressed = false;
    } else if (m_is_hovered) {
        m_shape.setFillColor(m_hover_color);
        m_shape.setOutlineColor(sf::Color(150, 230, 255, 255));
        m_inner_shape.setOutlineColor(sf::Color(180, 240, 255, 200));
        float pulse = std::sin(m_hover_time * 4.0f) * 0.5f + 0.5f;
        for (size_t i = 0; i < m_glow_layers.size(); ++i) {
            sf::Uint8 alpha = static_cast<sf::Uint8>((150 - static_cast<float>(i) * 40.0f) * pulse);
            m_glow_layers[i].setOutlineColor(sf::Color(100, 200, 255, alpha));
        }
        for (auto& dot : m_corner_dots) {
            sf::Uint8 dot_alpha = static_cast<sf::Uint8>(255 * pulse);
            dot.setFillColor(sf::Color(150, 230, 255, dot_alpha));
        }
        for (auto& line : m_side_lines) {
            line.setFillColor(sf::Color(150, 230, 255, 255));
        }
        float scan_y = std::fmod(m_scan_time * 30.0f, m_size.y);
        m_scan_line.setPosition(m_position + sf::Vector2f(15.0f, scan_y));
        sf::Uint8 scan_alpha = static_cast<sf::Uint8>(150 * pulse);
        m_scan_line.setFillColor(sf::Color(150, 230, 255, scan_alpha));
        m_text.setFillColor(sf::Color(255, 255, 255));
    } else {
        m_shape.setFillColor(m_normal_color);
        m_shape.setOutlineColor(sf::Color(100, 200, 255, 200));
        m_inner_shape.setOutlineColor(sf::Color(100, 200, 255, 100));
        for (auto& glow : m_glow_layers) {
            glow.setOutlineColor(sf::Color(100, 200, 255, 0));
        }
        for (auto& dot : m_corner_dots) {
            dot.setFillColor(sf::Color(100, 200, 255, 150));
        }
        for (auto& line : m_side_lines) {
            line.setFillColor(sf::Color(100, 200, 255, 180));
        }
        m_scan_line.setFillColor(sf::Color(100, 220, 255, 0));
        m_text.setFillColor(sf::Color(220, 220, 220));
    }
}

void Button::render(sf::RenderWindow& window) {
    if (m_is_hovered) {
        for (auto& glow : m_glow_layers) {
            window.draw(glow);
        }
    }
    window.draw(m_shape);
    window.draw(m_inner_shape);
    for (auto& line : m_side_lines) {
        window.draw(line);
    }
    for (auto& dot : m_corner_dots) {
        window.draw(dot);
    }
    if (m_is_hovered) {
        window.draw(m_scan_line);
    }
    window.draw(m_text);
}

bool Button::contains(const sf::Vector2f& point) const {
    sf::Vector2f local_point = point - m_position;
    if (local_point.x < 0 || local_point.x > m_size.x ||
        local_point.y < 0 || local_point.y > m_size.y) {
        return false;
    }
    const float bevel = 15.0f;
    if (local_point.y < m_size.y / 2.0f) {
        if (local_point.x < bevel && local_point.y < local_point.x * (m_size.y / 2.0f) / bevel) {
            return false;
        }
        if (local_point.x > m_size.x - bevel &&
            local_point.y < (m_size.x - local_point.x) * (m_size.y / 2.0f) / bevel) {
            return false;
        }
    } else {
        if (local_point.x < bevel &&
            (m_size.y - local_point.y) < local_point.x * (m_size.y / 2.0f) / bevel) {
            return false;
        }
        if (local_point.x > m_size.x - bevel &&
            (m_size.y - local_point.y) < (m_size.x - local_point.x) * (m_size.y / 2.0f) / bevel) {
            return false;
        }
    }
    return true;
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
    if (m_logo_texture.loadFromFile("assets/logoR-type1.png")) {
        m_has_logo = true;
        m_logo_sprite.setTexture(m_logo_texture);
        sf::FloatRect logo_bounds = m_logo_sprite.getLocalBounds();
        m_logo_sprite.setOrigin(logo_bounds.width / 2.0f, logo_bounds.height / 2.0f);
        m_logo_sprite.setScale(0.7f, 0.7f);
        m_logo_sprite.setPosition(position);
    }
    if (!m_font.loadFromFile("assets/fonts/arial.ttf")) {
        std::cerr << "Warning: Could not load font for title\n";
    }

    m_text.setFont(m_font);
    m_text.setString(text);
    m_text.setCharacterSize(size);
    m_text.setFillColor(sf::Color(100, 200, 255));
    m_text.setStyle(sf::Text::Bold);
    m_text.setOutlineThickness(3.0f);
    m_text.setOutlineColor(sf::Color(0, 100, 200));

    m_shadow.setFont(m_font);
    m_shadow.setString(text);
    m_shadow.setCharacterSize(size);
    m_shadow.setFillColor(sf::Color(0, 50, 100, 150));
    m_shadow.setStyle(sf::Text::Bold);

    sf::FloatRect bounds = m_text.getLocalBounds();
    m_text.setOrigin(bounds.left + bounds.width / 2.0f,
                     bounds.top + bounds.height / 2.0f);
    m_text.setPosition(position);

    m_shadow.setOrigin(bounds.left + bounds.width / 2.0f,
                       bounds.top + bounds.height / 2.0f);
    m_shadow.setPosition(position.x + 5.0f, position.y + 5.0f);

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> angle_dist(0.0f, 6.28f);
    std::uniform_real_distribution<float> dist_dist(120.0f, 200.0f);

    if (!m_has_logo) {
        for (int i = 0; i < 12; ++i) {
            float angle = angle_dist(gen);
            float distance = dist_dist(gen);
            sf::CircleShape particle(3.0f);
            particle.setFillColor(sf::Color(100, 200, 255, 120));
            particle.setPosition(
                position.x + std::cos(angle) * distance,
                position.y + std::sin(angle) * distance
            );
            m_particles.push_back(particle);
        }
    }
}

void MenuTitle::update(float dt) {
    m_glow_time += dt;
    float glow = std::sin(m_glow_time * 2.0f) * 0.3f + 0.7f;

    if (m_has_logo) {
        float pulse = std::sin(m_glow_time * 1.5f) * 0.005f + 1.0f;
        sf::FloatRect logo_bounds = m_logo_sprite.getLocalBounds();
        float base_scale = 0.7f;
        m_logo_sprite.setScale(base_scale * pulse, base_scale * pulse);
    }
    m_text.setFillColor(sf::Color(
        static_cast<sf::Uint8>(100 * glow),
        static_cast<sf::Uint8>(200 * glow),
        255
    ));

    sf::Vector2f center_pos = m_has_logo ? m_logo_sprite.getPosition() : m_text.getPosition();
    for (auto& particle : m_particles) {
        float current_x = particle.getPosition().x;
        float current_y = particle.getPosition().y;
        float angle = std::atan2(current_y - center_pos.y, current_x - center_pos.x);
        angle += dt * 0.5f;
        float distance = std::sqrt(
            (current_x - center_pos.x) * (current_x - center_pos.x) +
            (current_y - center_pos.y) * (current_y - center_pos.y)
        );
        particle.setPosition(
            center_pos.x + std::cos(angle) * distance,
            center_pos.y + std::sin(angle) * distance
        );
        sf::Uint8 particle_alpha = static_cast<sf::Uint8>(120 * glow);
        particle.setFillColor(sf::Color(100, 200, 255, particle_alpha));
    }
}

void MenuTitle::render(sf::RenderWindow& window) {
    for (const auto& particle : m_particles) {
        window.draw(particle);
    }
    if (m_has_logo) {
        window.draw(m_logo_sprite);
    } else {
        window.draw(m_shadow);
        window.draw(m_text);
    }
}

void MenuTitle::set_text(const std::string& text) { m_text.setString(text); }

MenuBackground::MenuBackground(const sf::Vector2u& window_size) : m_window_size(window_size) {
    m_background.setSize(sf::Vector2f(static_cast<float>(window_size.x),
                                      static_cast<float>(window_size.y)));
    m_background.setFillColor(sf::Color(5, 5, 15));

    if (m_background_texture.loadFromFile("assets/galaxie.jpg")) {
        m_background_sprite.setTexture(m_background_texture);
        sf::Vector2u tex_size = m_background_texture.getSize();
        float scale_x = static_cast<float>(window_size.x) / static_cast<float>(tex_size.x);
        float scale_y = static_cast<float>(window_size.y) / static_cast<float>(tex_size.y);
        float scale = std::max(scale_x, scale_y);
        m_background_sprite.setScale(scale, scale);
        float offset_x = (static_cast<float>(window_size.x) - static_cast<float>(tex_size.x) * scale) / 2.0f;
        float offset_y = (static_cast<float>(window_size.y) - static_cast<float>(tex_size.y) * scale) / 2.0f;
        m_background_sprite.setPosition(offset_x, offset_y);
        sf::Color tint = sf::Color::White;
        tint.a = 200;
        m_background_sprite.setColor(tint);
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> x_dist(0.0f, static_cast<float>(window_size.x));
    std::uniform_real_distribution<float> y_dist(0.0f, static_cast<float>(window_size.y));
    std::uniform_real_distribution<float> speed_dist(20.0f, 100.0f);
    std::uniform_real_distribution<float> size_dist(1.0f, 3.0f);

    for (int i = 0; i < 200; ++i) {
        Star star;
        star.position = sf::Vector2f(x_dist(gen), y_dist(gen));
        star.speed = speed_dist(gen);
        star.size = size_dist(gen);
        sf::Uint8 brightness = static_cast<sf::Uint8>(150 + (i % 100));
        star.color = sf::Color(brightness, brightness, 255, 200);
        m_stars.push_back(star);
    }

    m_ship_textures.resize(2);
    m_ship_textures.reserve(2);
    if (!m_ship_textures[0].loadFromFile("assets/r-typesheet1.png") ||
        !m_ship_textures[1].loadFromFile("assets/r-typesheet26.png")) {
        std::cerr << "Warning: Failed to load ship textures\n";
        return;
    }

    std::uniform_real_distribution<float> scale_dist(0.8f, 1.8f);
    std::uniform_real_distribution<float> speed_x_dist(80.0f, 200.0f);
    std::uniform_real_distribution<float> speed_y_dist(-30.0f, 30.0f);
    std::uniform_real_distribution<float> rot_dist(-0.2f, 0.2f);
    std::uniform_int_distribution<int> type_dist(0, 1);
    std::uniform_int_distribution<int> alpha_dist(150, 255);

    for (int i = 0; i < 6; ++i) {
        AnimatedShip ship;
        int type = type_dist(gen);
        ship.sprite.setTexture(m_ship_textures[type]);
        if (type == 0) {
            ship.frames.push_back(sf::IntRect(99, 0, 33, 17));
            ship.frames.push_back(sf::IntRect(132, 0, 33, 17));
            ship.frames.push_back(sf::IntRect(165, 0, 33, 17));
        } else {
            ship.frames.push_back(sf::IntRect(0, 0, 65, 50));
            ship.frames.push_back(sf::IntRect(65, 0, 65, 50));
            ship.frames.push_back(sf::IntRect(130, 0, 65, 50));
        }
        ship.current_frame = 0;
        ship.frame_duration = 0.12f;
        ship.time_accumulator = 0.0f;
        ship.sprite.setTextureRect(ship.frames[0]);
        auto rect = ship.frames[0];
        ship.sprite.setOrigin(static_cast<float>(rect.width) / 2.0f,
                              static_cast<float>(rect.height) / 2.0f);
        ship.scale = scale_dist(gen);
        ship.sprite.setScale(ship.scale, ship.scale);
        std::uniform_real_distribution<float> start_x_dist(-200.0f, static_cast<float>(window_size.x));
        ship.position = sf::Vector2f(start_x_dist(gen), y_dist(gen));
        ship.sprite.setPosition(ship.position);
        ship.velocity = sf::Vector2f(speed_x_dist(gen), speed_y_dist(gen));
        ship.rotation_speed = rot_dist(gen);
        sf::Uint8 alpha = static_cast<sf::Uint8>(alpha_dist(gen));
        ship.sprite.setColor(sf::Color(255, 255, 255, alpha));
        m_ships.push_back(ship);
    }

    std::vector<std::string> asteroid_files = {
        "assets/Asteroids/Asteroid_1.png",
        "assets/Asteroids/Asteroid_2.png",
        "assets/Asteroids/Asteroid_3.png",
        "assets/Asteroids/Asteroid_4.png",
        "assets/Asteroids/Asteroid_5.png"
    };

    m_asteroid_textures.reserve(asteroid_files.size());
    for (const auto& file : asteroid_files) {
        sf::Texture tex;
        if (tex.loadFromFile(file)) {
            m_asteroid_textures.push_back(std::move(tex));
        } else {
            std::cerr << "Warning: Could not load asteroid texture: " << file << "\n";
        }
    }

    if (!m_asteroid_textures.empty()) {
        std::uniform_real_distribution<float> asteroid_scale_dist(0.08f, 0.15f);
        std::uniform_real_distribution<float> asteroid_speed_x_dist(15.0f, 40.0f);
        std::uniform_real_distribution<float> asteroid_speed_y_dist(-8.0f, 8.0f);
        std::uniform_real_distribution<float> asteroid_rot_dist(-0.2f, 0.2f);
        std::uniform_int_distribution<int> asteroid_alpha_dist(100, 180);
        std::uniform_int_distribution<int> texture_dist(0, static_cast<int>(m_asteroid_textures.size()) - 1);

        for (int i = 0; i < 8; ++i) {
            Asteroid asteroid;
            int tex_idx = texture_dist(gen);
            asteroid.sprite.setTexture(m_asteroid_textures[tex_idx]);
            auto tex_size = m_asteroid_textures[tex_idx].getSize();
            asteroid.sprite.setOrigin(static_cast<float>(tex_size.x) / 2.0f,
                                     static_cast<float>(tex_size.y) / 2.0f);
            asteroid.scale = asteroid_scale_dist(gen);
            asteroid.sprite.setScale(asteroid.scale, asteroid.scale);
            std::uniform_real_distribution<float> start_x_dist(-150.0f, static_cast<float>(window_size.x));
            asteroid.position = sf::Vector2f(start_x_dist(gen), y_dist(gen));
            asteroid.sprite.setPosition(asteroid.position);
            asteroid.velocity = sf::Vector2f(asteroid_speed_x_dist(gen), asteroid_speed_y_dist(gen));
            asteroid.rotation = 0.0f;
            asteroid.rotation_speed = asteroid_rot_dist(gen);
            sf::Uint8 alpha = static_cast<sf::Uint8>(asteroid_alpha_dist(gen));
            asteroid.sprite.setColor(sf::Color(255, 255, 255, alpha));
            m_asteroids.push_back(asteroid);
        }
    }
}

void MenuBackground::update(float dt) {
    m_time += dt;

    for (auto& star : m_stars) {
        star.position.x -= star.speed * dt;
        if (star.position.x < 0.0f) {
            star.position.x = m_background.getSize().x;
        }
        float twinkle = std::sin(m_time * 3.0f + star.position.y) * 0.3f + 0.7f;
        sf::Uint8 alpha = static_cast<sf::Uint8>(200 * twinkle);
        star.color.a = alpha;
    }

    for (auto& ship : m_ships) {
        if (!ship.frames.empty()) {
            ship.time_accumulator += dt;
            if (ship.time_accumulator >= ship.frame_duration) {
                ship.time_accumulator -= ship.frame_duration;
                ship.current_frame = (ship.current_frame + 1) % ship.frames.size();
                ship.sprite.setTextureRect(ship.frames[ship.current_frame]);
            }
        }

        ship.position += ship.velocity * dt;
        ship.sprite.setPosition(ship.position);
        ship.sprite.rotate(ship.rotation_speed * dt * 10.0f);

        if (ship.position.x > static_cast<float>(m_window_size.x) + 200.0f) {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_real_distribution<float> y_dist(0.0f, static_cast<float>(m_window_size.y));
            std::uniform_real_distribution<float> speed_x_dist(80.0f, 200.0f);
            std::uniform_real_distribution<float> speed_y_dist(-30.0f, 30.0f);
            ship.position = sf::Vector2f(-200.0f, y_dist(gen));
            ship.velocity = sf::Vector2f(speed_x_dist(gen), speed_y_dist(gen));
            ship.sprite.setRotation(0.0f);
            ship.current_frame = 0;
            ship.time_accumulator = 0.0f;
        }
    }

    for (auto& asteroid : m_asteroids) {
        asteroid.position += asteroid.velocity * dt;
        asteroid.rotation += asteroid.rotation_speed * dt;
        asteroid.sprite.setPosition(asteroid.position);
        asteroid.sprite.setRotation(asteroid.rotation * 57.3f);

        if (asteroid.position.x > static_cast<float>(m_window_size.x) + 200.0f) {
            std::random_device rd;
            std::mt19937 gen(rd());
            std::uniform_real_distribution<float> y_dist(0.0f, static_cast<float>(m_window_size.y));
            std::uniform_real_distribution<float> speed_x_dist(15.0f, 40.0f);
            std::uniform_real_distribution<float> speed_y_dist(-8.0f, 8.0f);
            asteroid.position = sf::Vector2f(-200.0f, y_dist(gen));
            asteroid.velocity = sf::Vector2f(speed_x_dist(gen), speed_y_dist(gen));
            asteroid.rotation = 0.0f;
        }
    }
}

void MenuBackground::render(sf::RenderWindow& window) {
    window.draw(m_background);
    if (m_background_texture.getSize().x > 0) {
        window.draw(m_background_sprite);
    }

    for (const auto& asteroid : m_asteroids) {
        window.draw(asteroid.sprite);
    }

    for (const auto& ship : m_ships) {
        window.draw(ship.sprite);
    }

    for (const auto& star : m_stars) {
        sf::CircleShape star_shape(star.size);
        star_shape.setPosition(star.position);
        star_shape.setFillColor(star.color);
        window.draw(star_shape);
    }
}

ParticleEffect::ParticleEffect(const sf::Vector2f& position) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> angle_dist(0.0f, 6.28f);
    std::uniform_real_distribution<float> speed_dist(50.0f, 150.0f);

    for (int i = 0; i < 20; ++i) {
        Particle p;
        float angle = angle_dist(gen);
        float speed = speed_dist(gen);
        p.position = position;
        p.velocity = sf::Vector2f(std::cos(angle) * speed, std::sin(angle) * speed);
        p.lifetime = 0.0f;
        p.max_lifetime = 1.0f;
        p.color = sf::Color(100, 200, 255);
        m_particles.push_back(p);
    }
}

void ParticleEffect::update(float dt) {
    for (auto& p : m_particles) {
        p.position += p.velocity * dt;
        p.velocity.y += 100.0f * dt;
        p.lifetime += dt;
        float alpha_factor = 1.0f - (p.lifetime / p.max_lifetime);
        p.color.a = static_cast<sf::Uint8>(255 * alpha_factor);
    }
}

void ParticleEffect::render(sf::RenderWindow& window) {
    for (const auto& p : m_particles) {
        sf::CircleShape particle_shape(2.0f);
        particle_shape.setPosition(p.position);
        particle_shape.setFillColor(p.color);
        window.draw(particle_shape);
    }
}

bool ParticleEffect::is_alive() const {
    if (m_particles.empty()) return false;
    return m_particles[0].lifetime < m_particles[0].max_lifetime;
}

MenuFooter::MenuFooter(const sf::Vector2u& window_size) {
    if (!m_font.loadFromFile("assets/fonts/arial.ttf")) {
        std::cerr << "Warning: Could not load font for footer\n";
    }

    m_version_text.setFont(m_font);
    m_version_text.setString("R-TYPE v1.0.0");
    m_version_text.setCharacterSize(16);
    m_version_text.setFillColor(sf::Color(100, 150, 200, 180));
    m_version_text.setPosition(20.0f, static_cast<float>(window_size.y) - 30.0f);

    m_copyright_text.setFont(m_font);
    m_copyright_text.setString("(C) 2025 - Epitech Project");
    m_copyright_text.setCharacterSize(14);
    m_copyright_text.setFillColor(sf::Color(80, 120, 160, 150));
    sf::FloatRect bounds = m_copyright_text.getLocalBounds();
    m_copyright_text.setPosition(
        static_cast<float>(window_size.x) - bounds.width - 20.0f,
        static_cast<float>(window_size.y) - 30.0f
    );

    sf::RectangleShape line;
    line.setSize(sf::Vector2f(static_cast<float>(window_size.x) - 40.0f, 1.0f));
    line.setPosition(20.0f, static_cast<float>(window_size.y) - 50.0f);
    line.setFillColor(sf::Color(100, 150, 200, 100));
    m_decorative_lines.push_back(line);
}

void MenuFooter::render(sf::RenderWindow& window) {
    for (const auto& line : m_decorative_lines) {
        window.draw(line);
    }
    window.draw(m_version_text);
    window.draw(m_copyright_text);
}

CornerDecoration::CornerDecoration(const sf::Vector2f& position, bool flip_x, bool flip_y) {
    float x_mult = flip_x ? -1.0f : 1.0f;
    float y_mult = flip_y ? -1.0f : 1.0f;

    sf::RectangleShape line1(sf::Vector2f(80.0f, 2.0f));
    line1.setPosition(position.x, position.y);
    line1.setFillColor(sf::Color(100, 200, 255, 150));
    if (flip_x) line1.setPosition(position.x - 80.0f, position.y);
    m_lines.push_back(line1);

    sf::RectangleShape line2(sf::Vector2f(2.0f, 80.0f));
    line2.setPosition(position.x, position.y);
    line2.setFillColor(sf::Color(100, 200, 255, 150));
    if (flip_y) line2.setPosition(position.x, position.y - 80.0f);
    m_lines.push_back(line2);

    for (int i = 0; i < 3; ++i) {
        sf::CircleShape dot(2.0f);
        dot.setFillColor(sf::Color(100, 200, 255, 200));
        dot.setPosition(
            position.x + x_mult * static_cast<float>(i * 25 + 10),
            position.y + y_mult * static_cast<float>(i * 25 + 10)
        );
        m_dots.push_back(dot);
    }
}

void CornerDecoration::update(float dt) {
    m_glow_time += dt;
    float glow = std::sin(m_glow_time * 2.0f) * 0.3f + 0.7f;

    for (auto& line : m_lines) {
        sf::Uint8 alpha = static_cast<sf::Uint8>(150 * glow);
        sf::Color color = line.getFillColor();
        color.a = alpha;
        line.setFillColor(color);
    }

    for (auto& dot : m_dots) {
        sf::Uint8 alpha = static_cast<sf::Uint8>(200 * glow);
        sf::Color color = dot.getFillColor();
        color.a = alpha;
        dot.setFillColor(color);
    }
}

void CornerDecoration::render(sf::RenderWindow& window) {
    for (const auto& line : m_lines) {
        window.draw(line);
    }
    for (const auto& dot : m_dots) {
        window.draw(dot);
    }
}

SidePanel::SidePanel(const sf::Vector2f& position, bool is_left) {
    if (!m_font.loadFromFile("assets/fonts/arial.ttf")) {
        std::cerr << "Warning: Could not load font for side panel\n";
    }

    m_label.setFont(m_font);
    m_label.setString(is_left ? "SYS" : "PWR");
    m_label.setCharacterSize(14);
    m_label.setFillColor(sf::Color(100, 200, 255, 150));
    m_label.setPosition(position + sf::Vector2f(is_left ? 10.0f : -30.0f, 10.0f));

    for (int i = 0; i < 5; ++i) {
        sf::RectangleShape bar;
        float height = 40.0f + static_cast<float>(i * 15);
        bar.setSize(sf::Vector2f(4.0f, height));
        bar.setPosition(position + sf::Vector2f(is_left ? 15.0f : -10.0f, 40.0f + static_cast<float>(i * 25)));
        bar.setFillColor(sf::Color(50, 150, 255, static_cast<sf::Uint8>(100 + i * 20)));
        m_bars.push_back(bar);

        sf::CircleShape indicator(3.0f);
        indicator.setPosition(position + sf::Vector2f(is_left ? 12.0f : -16.0f, 35.0f + static_cast<float>(i * 25)));
        indicator.setFillColor(sf::Color(100, 200, 255, 200));
        m_indicators.push_back(indicator);
    }
}

void SidePanel::update(float dt) {
    m_anim_time += dt;

    for (size_t i = 0; i < m_bars.size(); ++i) {
        float wave = std::sin(m_anim_time * 2.0f + static_cast<float>(i) * 0.5f) * 0.3f + 0.7f;
        sf::Uint8 alpha = static_cast<sf::Uint8>((100.0f + static_cast<float>(i) * 20.0f) * wave);
        sf::Color color = m_bars[i].getFillColor();
        color.a = alpha;
        m_bars[i].setFillColor(color);
        float pulse = std::sin(m_anim_time * 3.0f + static_cast<float>(i)) * 0.5f + 0.5f;
        sf::Uint8 indicator_alpha = static_cast<sf::Uint8>(200 * pulse);
        m_indicators[i].setFillColor(sf::Color(100, 200, 255, indicator_alpha));
    }
}

void SidePanel::render(sf::RenderWindow& window) {
    window.draw(m_label);
    for (const auto& bar : m_bars) {
        window.draw(bar);
    }
    for (const auto& indicator : m_indicators) {
        window.draw(indicator);
    }
}

} // namespace rtype::ui
