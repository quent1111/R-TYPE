#include "common/SafeQueue.hpp"
#include "network/Messages.hpp"
#include "network/NetworkClient.hpp"
#include "states/GameState.hpp"
#include "states/LobbyState.hpp"
#include "states/LobbyListState.hpp"
#include "states/MenuState.hpp"
#include "states/StateManager.hpp"

#include <SFML/Graphics.hpp>

#include <iostream>
#include <memory>
#include <thread>

int main(int argc, char* argv[]) {
    sf::RenderWindow window(sf::VideoMode(1920, 1080), "R-TYPE - Multiplayer");
    window.setVerticalSyncEnabled(false);
    window.setFramerateLimit(60);

    try {
        std::string host = "127.0.0.1";
        unsigned short port = 4242;

        for (int i = 1; i < argc; ++i) {
            std::string arg = argv[i];
            if (arg == "-h" && i + 1 < argc) {
                host = argv[++i];
            } else if (arg == "-p" && i + 1 < argc) {
                try {
                    port = static_cast<unsigned short>(std::stoul(argv[++i]));
                } catch (...) {
                    std::cerr << "[main] Port invalide, utilisation du port par défaut 4242"
                              << std::endl;
                    port = 4242;
                }
            } else if (!arg.empty() && arg[0] != '-') {
                if (host == "127.0.0.1") {
                    host = arg;
                } else {
                    try {
                        port = static_cast<unsigned short>(std::stoul(arg));
                    } catch (...) {}
                }
            }
        }

        std::cout << "[main] Connecting to server " << host << ":" << port << std::endl;
        auto game_to_network_queue = std::make_shared<ThreadSafeQueue<GameToNetwork::Message>>();
        auto network_to_game_queue = std::make_shared<ThreadSafeQueue<NetworkToGame::Message>>();

        std::cout << "[main] Connecting to server..." << std::endl;
        auto network_client = std::make_shared<NetworkClient>(host, port, *game_to_network_queue,
                                                              *network_to_game_queue);
        std::cout << "[main] NetworkClient connected and running.\n";

        rtype::StateManager state_manager;

        state_manager.register_state(
            "menu", [&window]() { return std::make_unique<rtype::MenuState>(window); });

        state_manager.register_state("lobby_list",
                                     [&window, game_to_network_queue, network_to_game_queue]() {
                                         return std::make_unique<rtype::LobbyListState>(
                                             window, game_to_network_queue, network_to_game_queue);
                                     });

        state_manager.register_state("lobby",
                                     [&window, game_to_network_queue, network_to_game_queue]() {
                                         return std::make_unique<rtype::LobbyState>(
                                             window, game_to_network_queue, network_to_game_queue);
                                     });

        state_manager.register_state("game",
                                     [&window, game_to_network_queue, network_to_game_queue]() {
                                         return std::make_unique<rtype::GameState>(
                                             window, game_to_network_queue, network_to_game_queue);
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
        }

    } catch (const std::exception& e) {
        std::cerr << "[Fatal Error] " << e.what() << std::endl;
        return 84;
    }

    return 0;
}