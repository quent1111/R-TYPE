#include "components.hpp"
#include "registry.hpp"
#include "systems.hpp"

#include <SFML/Graphics.hpp>

#include <iostream>

int main() {
    sf::RenderWindow window(sf::VideoMode(800, 600), "test_ecs");
    window.setFramerateLimit(60);

    // Create registry and register all components
    registry reg;
    reg.register_component<position>();
    reg.register_component<velocity>();
    reg.register_component<drawable>();
    reg.register_component<controllable>();

    // Create a controllable player entity (green square)
    auto player = reg.spawn_entity();
    reg.emplace_component<position>(player, 400.0f, 300.0f);
    reg.emplace_component<velocity>(player, 0.0f, 0.0f);
    reg.emplace_component<drawable>(player, sf::Vector2f(50.0f, 50.0f), sf::Color::Green);
    reg.emplace_component<controllable>(player, 200.0f);

    // Create some non-controllable entities (obstacles)
    auto obstacle1 = reg.spawn_entity();
    reg.emplace_component<position>(obstacle1, 100.0f, 100.0f);
    reg.emplace_component<drawable>(obstacle1, sf::Vector2f(80.0f, 80.0f), sf::Color::Red);

    auto obstacle2 = reg.spawn_entity();
    reg.emplace_component<position>(obstacle2, 600.0f, 400.0f);
    reg.emplace_component<drawable>(obstacle2, sf::Vector2f(60.0f, 60.0f), sf::Color::Blue);

    // Create a moving entity (no control, just velocity)
    auto moving_obj = reg.spawn_entity();
    reg.emplace_component<position>(moving_obj, 200.0f, 500.0f);
    reg.emplace_component<velocity>(moving_obj, 50.0f, -30.0f);
    reg.emplace_component<drawable>(moving_obj, sf::Vector2f(40.0f, 40.0f), sf::Color::Yellow);

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
        control_system(reg);       // Handle input
        position_system(reg, dt);  // Update positions (with delta time)
        // Rendering
        window.clear(sf::Color::Black);
        draw_system(reg, window);  // Draw everything
        window.display();
    }

    return 0;
}
