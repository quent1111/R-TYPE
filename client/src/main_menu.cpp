#include "states/GameState.hpp"
#include "states/MenuState.hpp"
#include "states/StateManager.hpp"

#include <SFML/Graphics.hpp>
#include <iostream>

int main(int, char**) {
    sf::RenderWindow window(sf::VideoMode(1920, 1080), "R-Type");
    window.setFramerateLimit(60);

    rtype::StateManager state_manager;

    // Register all states
    state_manager.register_state("menu", [&window]() {
        return std::make_unique<rtype::MenuState>(window);
    });

    state_manager.register_state("game", [&window]() {
        return std::make_unique<rtype::GameState>(window);
    });

    // Start with menu
    state_manager.push_state("menu");

    sf::Clock clock;

    // Main loop
    while (window.isOpen() && state_manager.has_states()) {
        float dt = clock.restart().asSeconds();

        // Handle events
        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            state_manager.handle_event(event);
        }

        // Update
        state_manager.update(dt);

        // Process state transitions
        state_manager.process_transitions();

        // Render
        window.clear(sf::Color(10, 10, 30));
        state_manager.render(window);
        window.display();
    }

    std::cout << "R-Type exiting...\n";
    return 0;
}
