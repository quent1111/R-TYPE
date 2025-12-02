#include <iostream>
#include <exception>
#include <thread>
#include <memory>
#include "Game.hpp"
#include "NetworkClient.hpp"
#include "SafeQueue.hpp"
#include "Messages.hpp"

int main() {
    try {
        ThreadSafeQueue<GameToNetwork::Message> game_to_network_queue;
        ThreadSafeQueue<NetworkToGame::Message> network_to_game_queue;

        std::unique_ptr<NetworkClient> network_client;
        std::thread network_thread([&]() {
            try {
                network_client = std::make_unique<NetworkClient>(
                    "localhost",
                    4242,
                    game_to_network_queue,
                    network_to_game_queue
                );
                while (network_client) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }
            } catch (const std::exception& e) {
                std::cerr << "[Network Thread Error] " << e.what() << std::endl;
            }
        });

        std::this_thread::sleep_for(std::chrono::milliseconds(100));

        Game game(game_to_network_queue, network_to_game_queue);
        game.run();

        network_client.reset();
        if (network_thread.joinable()) {
            network_thread.join();
        }
    } catch (const std::exception& e) {
        std::cerr << "[Error] " << e.what() << std::endl;
        return 1;
    }
    return 0;
}