#include "core/SettingsManager.hpp"
#include "states/GameState.hpp"
#include "states/MenuState.hpp"
#include "states/SettingsState.hpp"
#include "states/StateManager.hpp"

#include <SFML/Graphics.hpp>
#include <iostream>

int main(int, char**) {
    auto& settings = rtype::SettingsManager::get_instance();
    settings.load_from_file("settings.ini");

    sf::VideoMode mode = settings.get_resolution();
    sf::Uint32 style = settings.is_fullscreen() ? sf::Style::Fullscreen : sf::Style::Close;
    sf::RenderWindow window(mode, "R-Type", style);
    window.setVerticalSyncEnabled(settings.is_vsync_enabled());
    window.setFramerateLimit(60);

    std::cout << "[R-Type] Window created with resolution: "
              << mode.width << "x" << mode.height
              << " (Fullscreen: " << (settings.is_fullscreen() ? "ON" : "OFF") << ")\n";

    rtype::StateManager state_manager;

    state_manager.register_state("menu", [&window]() {
        return std::make_unique<rtype::MenuState>(window);
    });

    state_manager.register_state("settings", [&window]() {
        return std::make_unique<rtype::SettingsState>(window);
    });

    state_manager.register_state("game", [&window]() {
        return std::make_unique<rtype::GameState>(window);
    });

    state_manager.push_state("menu");

    sf::Clock clock;

    while (window.isOpen() && state_manager.has_states()) {
        float dt = clock.restart().asSeconds();

        sf::Event event;
        while (window.pollEvent(event)) {
            if (event.type == sf::Event::Closed) {
                window.close();
            }

            state_manager.handle_event(event);
        }

        state_manager.update(dt);

        state_manager.process_transitions();

        window.clear(sf::Color(10, 10, 30));
        state_manager.render(window);
        window.display();
    }

    std::cout << "R-Type exiting...\n";
    return 0;
}
