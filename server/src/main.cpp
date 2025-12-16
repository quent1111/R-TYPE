#include "game/GameSession.hpp"
#include "network/UDPServer.hpp"

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

void game_loop(server::UDPServer& server) {
    server::GameSession gameInstance;
    gameInstance.runGameLoop(server);
}

void network_loop(server::UDPServer& server) {
    std::cout << "[Core] Network loop started" << std::endl;
    server.run_network_loop();
    std::cout << "[Core] Network loop stopped" << std::endl;
}

int main(int argc, char** argv) {
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    std::string bind_address = "127.0.0.1";
    unsigned short port = 4242;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-h" && i + 1 < argc) {
            bind_address = argv[++i];
        } else if (arg == "-p" && i + 1 < argc) {
            try {
                port = static_cast<unsigned short>(std::stoul(argv[++i]));
            } catch (...) {
                std::cerr << "[Error] Invalid port specified, using default 4242" << std::endl;
                port = 4242;
            }
        } else {
            std::cerr << "[Error] Unknown argument: " << arg << std::endl;
        }
    }

    std::cout << "==============================" << std::endl;
    std::cout << "   R-TYPE Server Starting     " << std::endl;
    std::cout << "==============================" << std::endl;
    std::cout << "[Config] Bind address: " << bind_address << std::endl;
    std::cout << "[Config] Port: " << port << std::endl;

    try {
        asio::io_context io_context;
        server::UDPServer server(io_context, bind_address, port);

        std::thread network_thread(network_loop, std::ref(server));
        std::thread game_thread(game_loop, std::ref(server));

        game_thread.join();
        server.stop();
        network_thread.join();

        std::cout << "[Core] Server shutdown complete" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "[Fatal] " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
