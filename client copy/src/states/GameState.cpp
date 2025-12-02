#include "states/GameState.hpp"
#include "components/game_components.hpp"
#include "core/SettingsManager.hpp"
#include "core/FontManager.hpp"
#include "ecs/components.hpp"
#include "entities/enemy_factory.hpp"
#include "entities/player_factory.hpp"
#include "systems/cleanup_system.hpp"
#include "systems/collision_system.hpp"
#include "systems/input_system.hpp"
#include "systems/movement_system.hpp"
#include "systems/shooting_system.hpp"

#include <iostream>

namespace rtype {

GameState::GameState(sf::RenderWindow& window) : m_window(window) {}

void GameState::on_enter() {
    std::cout << "[GameState] Entering game\n";
    init_game();
}

void GameState::on_exit() {
    std::cout << "[GameState] Exiting game\n";
}

void GameState::init_game() {
    const sf::Font* font = core::FontManager::instance().get_default_font();
    if (!font) {
        std::cerr << "Warning: Could not load font for game UI\n";
    }

    if (!m_bg_texture.loadFromFile("assets/bg.png")) {
        std::cerr << "Warning: Could not load background texture\n";
    }
    m_bg_texture.setRepeated(true);

    createPlayer(m_registry, 200.0f, 540.0f);

    spawnEnemyWave(m_registry, 5);

    m_enemy_spawn_clock.restart();
    m_bg_scroll_offset = 0.0f;
}

void GameState::handle_event(const sf::Event& event) {
    if (event.type == sf::Event::KeyPressed) {
        if (event.key.code == sf::Keyboard::Escape) {
            std::cout << "[GameState] Escape pressed - going back to menu\n";
            m_next_state = "menu";
        }
    }
}

void GameState::update(float dt) {
    if (m_paused) {
        return;
    }

    if (m_enemy_spawn_clock.getElapsedTime().asSeconds() > m_enemy_spawn_interval) {
        spawnEnemyWave(m_registry, 3);
        m_enemy_spawn_clock.restart();
    }

    run_systems(dt);
    update_background(dt);
    update_fps(dt);
}

void GameState::render(sf::RenderWindow& window) {
    render_background(window);

    render_system(window);

    render_ui(window);
}


void GameState::update_background(float dt) {
    m_bg_scroll_offset += m_bg_scroll_speed * dt;
    const float window_width = static_cast<float>(m_window.getSize().x);
    if (m_bg_scroll_offset > window_width) {
        m_bg_scroll_offset -= window_width;
    }
}

void GameState::render_background(sf::RenderWindow& window) {
    const float window_width = static_cast<float>(window.getSize().x);
    const float window_height = static_cast<float>(window.getSize().y);

    sf::Sprite bg_sprite1(m_bg_texture);
    bg_sprite1.setTextureRect(
        sf::IntRect(0, 0, static_cast<int>(window_width), static_cast<int>(window_height)));
    bg_sprite1.setPosition(-m_bg_scroll_offset, 0);
    window.draw(bg_sprite1);

    sf::Sprite bg_sprite2(m_bg_texture);
    bg_sprite2.setTextureRect(
        sf::IntRect(0, 0, static_cast<int>(window_width), static_cast<int>(window_height)));
    bg_sprite2.setPosition(window_width - m_bg_scroll_offset, 0);
    window.draw(bg_sprite2);
}

void GameState::render_ui(sf::RenderWindow& window) {
    auto& healths = m_registry.get_components<health>();
    auto& player_tags = m_registry.get_components<player_tag>();

    for (std::size_t i = 0; i < healths.size() && i < player_tags.size(); ++i) {
        if (healths[i] && player_tags[i]) {
            auto& hp = healths[i].value();

            sf::RectangleShape health_bg(sf::Vector2f(200.0f, 20.0f));
            health_bg.setPosition(10.0f, 10.0f);
            health_bg.setFillColor(sf::Color(50, 50, 50));
            window.draw(health_bg);

            sf::RectangleShape health_bar(
                sf::Vector2f(200.0f * hp.health_percentage(), 20.0f));
            health_bar.setPosition(10.0f, 10.0f);
            health_bar.setFillColor(sf::Color(0, 255, 0));
            window.draw(health_bar);

            const sf::Font* font = core::FontManager::instance().get_default_font();
            if (font) {
                sf::Text health_text;
                health_text.setFont(*font);
                health_text.setString("HP: " + std::to_string(hp.current) + " / " +
                                      std::to_string(hp.maximum));
                health_text.setCharacterSize(18);
                health_text.setFillColor(sf::Color::White);
                health_text.setPosition(15.0f, 11.0f);
                window.draw(health_text);
            }

            break;
        }
    }
    auto& settings = SettingsManager::get_instance();
    if (settings.should_show_fps()) {
        const sf::Font* font = core::FontManager::instance().get_default_font();
        if (font) {
            sf::Text fps_text;
            fps_text.setFont(*font);
            fps_text.setString("FPS: " + std::to_string(static_cast<int>(m_fps)));
            fps_text.setCharacterSize(20);
            fps_text.setFillColor(sf::Color::Yellow);
            fps_text.setOutlineColor(sf::Color::Black);
            fps_text.setOutlineThickness(1.0f);
            sf::FloatRect bounds = fps_text.getLocalBounds();
            fps_text.setPosition(static_cast<float>(m_window.getSize().x) - bounds.width - 15.0f, 10.0f);
            window.draw(fps_text);
        }
    }
}


void GameState::run_systems(float dt) {
    inputSystem(m_registry);
    shootingSystem(m_registry, dt);
    movementSystem(m_registry, dt);
    collisionSystem(m_registry);
    explosion_system(dt);
    cleanupSystem(m_registry);
    animation_system(dt);
}

void GameState::explosion_system(float dt) {
    auto& explosion_tags = m_registry.get_components<explosion_tag>();

    for (std::size_t i = 0; i < explosion_tags.size(); ++i) {
        auto& explosion_opt = explosion_tags[i];
        if (explosion_opt) {
            explosion_opt.value().elapsed += dt;
        }
    }
}

void GameState::animation_system(float dt) {
    auto& animations = m_registry.get_components<animation_component>();
    auto& sprites = m_registry.get_components<sprite_component>();

    for (std::size_t i = 0; i < animations.size() && i < sprites.size(); ++i) {
        auto& anim_opt = animations[i];
        auto& sprite_opt = sprites[i];

        if (anim_opt && sprite_opt) {
            auto& anim = anim_opt.value();
            auto& sprite = sprite_opt.value();

            anim.update(dt);

            const auto frame = anim.get_current_frame();
            sprite.texture_rect_x = frame.left;
            sprite.texture_rect_y = frame.top;
            sprite.texture_rect_w = frame.width;
            sprite.texture_rect_h = frame.height;
        }
    }
}

void GameState::render_system(sf::RenderWindow& window) {
    auto& positions = m_registry.get_components<position>();
    auto& sprites = m_registry.get_components<sprite_component>();

    for (std::size_t i = 0; i < positions.size() && i < sprites.size(); ++i) {
        auto& pos_opt = positions[i];
        auto& sprite_opt = sprites[i];

        if (pos_opt && sprite_opt) {
            auto& pos = pos_opt.value();
            auto& sprite_comp = sprite_opt.value();

            if (sprite_comp.texture_path.empty()) {
                continue;
            }

            try {
                sf::Texture& texture = m_textures.load(sprite_comp.texture_path);
                texture.setSmooth(false);

                sf::Sprite sprite(texture);
                sprite.setTextureRect(sf::IntRect(sprite_comp.texture_rect_x,
                                                  sprite_comp.texture_rect_y,
                                                  sprite_comp.texture_rect_w,
                                                  sprite_comp.texture_rect_h));
                sprite.setScale(sprite_comp.scale, sprite_comp.scale);
                sprite.setPosition(
                    pos.x - (static_cast<float>(sprite_comp.texture_rect_w) *
                             sprite_comp.scale / 2.0f),
                    pos.y - (static_cast<float>(sprite_comp.texture_rect_h) *
                             sprite_comp.scale / 2.0f));

                window.draw(sprite);
            } catch (const std::runtime_error&) {
            }
        }
    }
}

void GameState::update_fps(float dt) {
    m_frame_count++;
    m_fps_update_time += dt;
    if (m_fps_update_time >= 0.5f) {
        m_fps = static_cast<float>(m_frame_count) / m_fps_update_time;
        m_frame_count = 0;
        m_fps_update_time = 0.0f;
    }
}

} // namespace rtype
