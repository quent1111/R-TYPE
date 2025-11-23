#include <SFML/Graphics.hpp>
#include <iostream>
#include "ecs/registry.hpp"
#include "ecs/components.hpp"
#include "components/game_components.hpp"
#include "systems/input_system.hpp"
#include "systems/movement_system.hpp"
#include "systems/shooting_system.hpp"
#include "systems/collision_system.hpp"
#include "systems/cleanup_system.hpp"
#include "entities/player_factory.hpp"
#include "entities/enemy_factory.hpp"
#include "texture_manager.hpp"

void explosionSystem(registry& reg, float dt) {
    auto& explosion_tags = reg.get_components<explosion_tag>();

    for (std::size_t i = 0; i < explosion_tags.size(); ++i) {
        auto& explosion_opt = explosion_tags[i];
        if (explosion_opt) {
            explosion_opt.value().elapsed += dt;
        }
    }
}

void animationSystem(registry& reg, float dt) {
    auto& animations = reg.get_components<animation_component>();
    auto& sprites = reg.get_components<sprite_component>();

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

void renderSystem(registry& reg, sf::RenderWindow& window, TextureManager& textures) {
    auto& positions = reg.get_components<position>();
    auto& sprites = reg.get_components<sprite_component>();

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
                sf::Texture& texture = textures.load(sprite_comp.texture_path);
                texture.setSmooth(false);

                sf::Sprite sprite(texture);
                sprite.setTextureRect(sf::IntRect(sprite_comp.texture_rect_x,
                                                   sprite_comp.texture_rect_y,
                                                   sprite_comp.texture_rect_w,
                                                   sprite_comp.texture_rect_h));
                sprite.setScale(sprite_comp.scale, sprite_comp.scale);
                sprite.setPosition(pos.x - (static_cast<float>(sprite_comp.texture_rect_w) * sprite_comp.scale / 2.0f),
                                   pos.y - (static_cast<float>(sprite_comp.texture_rect_h) * sprite_comp.scale / 2.0f));

                window.draw(sprite);
            } catch (const std::runtime_error& e) {
            }
        }
    }

}

void renderUI(registry& reg, sf::RenderWindow& window, sf::Font& font) {
    auto& healths = reg.get_components<health>();
    auto& player_tags = reg.get_components<player_tag>();

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

            sf::Text health_text;
            health_text.setFont(font);
            health_text.setString("HP: " + std::to_string(hp.current) + " / " +
                                  std::to_string(hp.maximum));
            health_text.setCharacterSize(18);
            health_text.setFillColor(sf::Color::White);
            health_text.setPosition(15.0f, 11.0f);
            window.draw(health_text);

            break;
        }
    }
}

int main(int, char**) {
    sf::RenderWindow window(sf::VideoMode(1920, 1080), "R-Type");
    window.setFramerateLimit(60);

    sf::Font font;
    font.loadFromFile("assets/fonts/arial.ttf");

    registry reg;
    TextureManager textures;

    sf::Texture bg_texture;
    bg_texture.loadFromFile("assets/bg.png");
    bg_texture.setRepeated(true);
    float bg_scroll_offset = 0.0f;
    const float bg_scroll_speed = 50.0f;
    const float window_width = 1920.0f;
    const float window_height = 1080.0f;

    createPlayer(reg, 200.0f, 540.0f);
    spawnEnemyWave(reg, 5);

    sf::Clock clock;
    sf::Clock enemy_spawn_clock;

    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();

        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }
            if (event.type == sf::Event::KeyPressed) {
                if (event.key.code == sf::Keyboard::Escape) {
                    window.close();
                }
            }
        }

        if (enemy_spawn_clock.getElapsedTime().asSeconds() > 3.0f) {
            spawnEnemyWave(reg, 3);
            enemy_spawn_clock.restart();
        }

        inputSystem(reg);
        shootingSystem(reg, dt);
        movementSystem(reg, dt);
        collisionSystem(reg);
        explosionSystem(reg, dt);
        cleanupSystem(reg);
        animationSystem(reg, dt);

        bg_scroll_offset += bg_scroll_speed * dt;
        if (bg_scroll_offset > window_width) {
            bg_scroll_offset -= window_width;
        }

        window.clear(sf::Color(10, 10, 30));

        sf::Sprite bg_sprite1(bg_texture);
        bg_sprite1.setTextureRect(sf::IntRect(0, 0, static_cast<int>(window_width), static_cast<int>(window_height)));
        bg_sprite1.setPosition(-bg_scroll_offset, 0);
        window.draw(bg_sprite1);

        sf::Sprite bg_sprite2(bg_texture);
        bg_sprite2.setTextureRect(sf::IntRect(0, 0, static_cast<int>(window_width), static_cast<int>(window_height)));
        bg_sprite2.setPosition(window_width - bg_scroll_offset, 0);
        window.draw(bg_sprite2);

        renderSystem(reg, window, textures);
        renderUI(reg, window, font);
        window.display();
    }

    return 0;
}
