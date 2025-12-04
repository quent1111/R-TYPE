#include "states/StateManager.hpp"
#include "states/MenuState.hpp"
#include "states/LobbyState.hpp"
#include "states/GameState.hpp"
#include "NetworkClient.hpp"
#include "SafeQueue.hpp"
#include "Messages.hpp"

#include <SFML/Graphics.hpp>
#include <iostream>
#include <memory>
#include <thread>

int main() {
    sf::RenderWindow window(sf::VideoMode(1920, 1080), "R-TYPE - Multiplayer");
    window.setFramerateLimit(60);

    auto game_to_network_queue = std::make_shared<ThreadSafeQueue<GameToNetwork::Message>>();
    auto network_to_game_queue = std::make_shared<ThreadSafeQueue<NetworkToGame::Message>>();
    std::shared_ptr<NetworkClient> network_client = nullptr;
    std::unique_ptr<std::thread> network_thread = nullptr;

    rtype::StateManager state_manager;

    state_manager.register_state("menu", [&window]() {
        return std::make_unique<rtype::MenuState>(window);
    });

    state_manager.register_state("lobby", [&window, &network_client, &network_thread, game_to_network_queue, network_to_game_queue]() {
        if (!network_client) {
            network_client = std::make_shared<NetworkClient>("127.0.0.1", 4242, *game_to_network_queue, *network_to_game_queue);
            std::cout << "[main] NetworkClient created and started\n";
        }
        return std::make_unique<rtype::LobbyState>(window, game_to_network_queue, network_to_game_queue);
    });

    state_manager.register_state("game", [&window, game_to_network_queue, network_to_game_queue]() {
        return std::make_unique<rtype::GameState>(window, game_to_network_queue, network_to_game_queue);
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

        window.clear();
        state_manager.render(window);
        window.display();
    }

    if (network_client) {
        std::cout << "[main] Stopping network client...\n";
        network_client->stop();
        network_client.reset();
    }

    return 0;
}

