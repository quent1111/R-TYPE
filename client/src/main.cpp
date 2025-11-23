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

// Simple rendering system (client-side only)
void renderSystem(registry& reg, sf::RenderWindow& window) {
    auto& positions = reg.get_components<position>();
    auto& sprites = reg.get_components<sprite_component>();

    for (std::size_t i = 0; i < positions.size() && i < sprites.size(); ++i) {
        auto& pos_opt = positions[i];
        auto& sprite_opt = sprites[i];

        if (pos_opt && sprite_opt) {
            auto& pos = pos_opt.value();
            auto& sprite = sprite_opt.value();

            // Create a simple colored rectangle as placeholder
            sf::RectangleShape shape(sf::Vector2f(sprite.width, sprite.height));
            shape.setPosition(pos.x, pos.y);
            shape.setFillColor(sf::Color(sprite.r, sprite.g, sprite.b, sprite.a));
            shape.setOrigin(sprite.width / 2.0f, sprite.height / 2.0f);

            window.draw(shape);
        }
    }
}

// Simple UI system
void renderUI(registry& reg, sf::RenderWindow& window, sf::Font& font) {
    auto& healths = reg.get_components<health>();
    auto& player_tags = reg.get_components<player_tag>();

    // Find player health
    for (std::size_t i = 0; i < healths.size() && i < player_tags.size(); ++i) {
        if (healths[i] && player_tags[i]) {
            auto& hp = healths[i].value();

            // Draw health bar
            sf::RectangleShape health_bg(sf::Vector2f(200.0f, 20.0f));
            health_bg.setPosition(10.0f, 10.0f);
            health_bg.setFillColor(sf::Color(50, 50, 50));
            window.draw(health_bg);

            sf::RectangleShape health_bar(
                sf::Vector2f(200.0f * hp.health_percentage(), 20.0f));
            health_bar.setPosition(10.0f, 10.0f);
            health_bar.setFillColor(sf::Color(0, 255, 0));
            window.draw(health_bar);

            // Draw health text
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

    // Draw controls info
    sf::Text controls;
    controls.setFont(font);
    controls.setString("WASD/Arrows: Move  |  Space: Shoot  |  ESC: Quit");
    controls.setCharacterSize(16);
    controls.setFillColor(sf::Color(200, 200, 200));
    controls.setPosition(10.0f, window.getSize().y - 30.0f);
    window.draw(controls);
}

int main(int argc, char* argv[]) {
    std::cout << "R-Type Client starting..." << std::endl;

    // Create window
    sf::RenderWindow window(sf::VideoMode(1920, 1080), "R-Type - Solo Demo");
    window.setFramerateLimit(60);

    // Load font (fallback to default if not found)
    sf::Font font;
    if (!font.loadFromFile("assets/fonts/arial.ttf")) {
        std::cerr << "Warning: Could not load font, UI text will not display" << std::endl;
    }

    // Create registry and setup game
    registry reg;

    std::cout << "Creating player..." << std::endl;
    entity player = createPlayer(reg, 200.0f, 540.0f);

    std::cout << "Spawning enemy wave..." << std::endl;
    spawnEnemyWave(reg, 5);

    std::cout << "Starting game loop..." << std::endl;

    sf::Clock clock;
    sf::Clock enemy_spawn_clock;

    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();

        // Handle events
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

        // Spawn new enemies periodically
        if (enemy_spawn_clock.getElapsedTime().asSeconds() > 3.0f) {
            spawnEnemyWave(reg, 3);
            enemy_spawn_clock.restart();
        }

        // Update game systems (order matters!)
        inputSystem(reg);
        shootingSystem(reg, dt);
        movementSystem(reg, dt);
        collisionSystem(reg);
        cleanupSystem(reg);

        // Render
        window.clear(sf::Color(10, 10, 30));  // Dark blue background
        renderSystem(reg, window);
        renderUI(reg, window, font);
        window.display();
    }

    std::cout << "Client shutting down..." << std::endl;
    return 0;
}
