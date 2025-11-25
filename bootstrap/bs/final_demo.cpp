#include "components.hpp"
#include "registry.hpp"
#include "systems.hpp"

#include <SFML/Graphics.hpp>

#include <iostream>

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "final demo");
    window.setFramerateLimit(60);

    // Create registry and register all components
    registry reg;
    reg.register_component<position>();
    reg.register_component<velocity>();
    reg.register_component<drawable>();
    reg.register_component<controllable>();
    reg.register_component<acceleration>();
    reg.register_component<looping>();

    // 1. FULLY MOVABLE ENTITY with acceleration (player with physics)
    auto player = reg.spawn_entity();
    reg.emplace_component<position>(player, 400.0f, 300.0f);
    reg.emplace_component<velocity>(player, 0.0f, 0.0f);
    reg.emplace_component<drawable>(player, sf::Vector2f(50.0f, 50.0f), sf::Color::Green);
    reg.emplace_component<controllable>(player, 500.0f);  // Acceleration rate
    reg.emplace_component<acceleration>(player, 0.0f, 0.0f, 400.0f,
                                        0.92f);  // max_speed=400, friction=0.92
    reg.emplace_component<looping>(player, 800.0f, 600.0f);

    // 2. STATIC ENTITIES (obstacles)
    auto obstacle1 = reg.spawn_entity();
    reg.emplace_component<position>(obstacle1, 100.0f, 100.0f);
    reg.emplace_component<drawable>(obstacle1, sf::Vector2f(80.0f, 80.0f), sf::Color::Red);

    auto obstacle2 = reg.spawn_entity();
    reg.emplace_component<position>(obstacle2, 600.0f, 400.0f);
    reg.emplace_component<drawable>(obstacle2, sf::Vector2f(60.0f, 60.0f), sf::Color::Blue);

    auto obstacle3 = reg.spawn_entity();
    reg.emplace_component<position>(obstacle3, 400.0f, 100.0f);
    reg.emplace_component<drawable>(obstacle3, sf::Vector2f(100.0f, 40.0f), sf::Color::White);

    // 3. LOOPING ENTITIES (move across screen and wrap around)
    auto looper1 = reg.spawn_entity();
    reg.emplace_component<position>(looper1, 0.0f, 250.0f);
    reg.emplace_component<velocity>(looper1, 100.0f, 0.0f);  // Moving right
    reg.emplace_component<drawable>(looper1, sf::Vector2f(40.0f, 40.0f), sf::Color::Yellow);
    reg.emplace_component<looping>(looper1, 800.0f, 600.0f);

    auto looper2 = reg.spawn_entity();
    reg.emplace_component<position>(looper2, 800.0f, 350.0f);
    reg.emplace_component<velocity>(looper2, -80.0f, 0.0f);  // Moving left
    reg.emplace_component<drawable>(looper2, sf::Vector2f(40.0f, 40.0f), sf::Color::Cyan);
    reg.emplace_component<looping>(looper2, 800.0f, 600.0f);

    auto looper3 = reg.spawn_entity();
    reg.emplace_component<position>(looper3, 400.0f, 0.0f);
    reg.emplace_component<velocity>(looper3, 0.0f, 120.0f);  // Moving down
    reg.emplace_component<drawable>(looper3, sf::Vector2f(40.0f, 40.0f), sf::Color::Magenta);
    reg.emplace_component<looping>(looper3, 800.0f, 600.0f);

    auto looper4 = reg.spawn_entity();
    reg.emplace_component<position>(looper4, 200.0f, 600.0f);
    reg.emplace_component<velocity>(looper4, 50.0f, -90.0f);  // Moving diagonal
    reg.emplace_component<drawable>(looper4, sf::Vector2f(40.0f, 40.0f),
                                    sf::Color(255, 128, 0));  // Orange
    reg.emplace_component<looping>(looper4, 800.0f, 600.0f);

    sf::Clock clock;

    // Main game loop
    while (window.isOpen()) {
        float dt = clock.restart().asSeconds();

        // Event handling
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed)
                window.close();
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Escape)
                window.close();
        }

        // Run systems in order
        acceleration_control_system(reg, dt);  // Handle acceleration-based control
        position_system(reg, dt);              // Update positions
        looping_system(reg);                   // Apply screen wrapping
        // Rendering
        window.clear(sf::Color::Black);
        draw_system(reg, window);  // Draw everything
        window.display();
    }

    return 0;
}
