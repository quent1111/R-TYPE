#include "NetworkPacket.hpp"
#include "UDPServer.hpp"
#include "Game.hpp"

#include <csignal>
#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>

std::atomic<bool> server_running{true};

void signal_handler(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        std::cout << "\nShutting down server..." << std::endl;
        server_running = false;
    }
}

void game_loop(UDPServer& server) {
    Game gameInstance;
    gameInstance.runGameLoop(server);
}

void network_loop(UDPServer& server) {
    std::cout << "[Core] Network loop started" << std::endl;
    server.run_network_loop();
    std::cout << "[Core] Network loop stopped" << std::endl;
}

int main(int argc, char** argv) {
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    unsigned short port = 4242;
    if (argc > 1) {
        port = static_cast<unsigned short>(std::stoi(argv[1]));
    }

    std::cout << "R-Type Server Starting..." << std::endl;
    std::cout << "Port: " << port << std::endl;

    try {
        asio::io_context io_context;
        UDPServer server(io_context, port);

        std::thread network_thread(network_loop, std::ref(server));
        std::thread game_thread(game_loop, std::ref(server));
        std::cout << "Server running... Press Ctrl+C to stop" << std::endl;
        game_thread.join();
        server.stop();
        network_thread.join();

        std::cout << "Server stopped successfully" << std::endl;
    } catch (const std::exception& e) {
        std::cerr << "Server error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
