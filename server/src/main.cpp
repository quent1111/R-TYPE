#include "game/ServerCore.hpp"
#include "network/UDPServer.hpp"
#include "powerup/PowerupRegistry.hpp"

#include <csignal>

#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>

#ifdef _WIN32
#include <winsock2.h>
#pragma comment(lib, "ws2_32.lib")
#endif

std::atomic<bool> server_running{true};

void signal_handler(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        std::cout << "\nShutting down server..." << std::endl;
        server_running = false;
    }
}

void game_loop(server::UDPServer& server) {
    server::ServerCore serverCore;
    serverCore.run_game_loop(server);
}

void network_loop(server::UDPServer& server) {
    std::cout << "[Core] Network loop started" << std::endl;
    server.run_network_loop();
    std::cout << "[Core] Network loop stopped" << std::endl;
}

int main(int argc, char** argv) {
#ifdef _WIN32
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        std::cerr << "[Fatal] WSAStartup failed: " << result << std::endl;
        return 1;
    }
    std::cout << "[Init] Windows Sockets initialized" << std::endl;
#endif

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

    std::cout << "[Init] Initializing power-up system..." << std::endl;
    powerup::PowerupRegistry::instance().initialize();

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
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }

#ifdef _WIN32
    WSACleanup();
    std::cout << "[Init] Windows Sockets cleaned up" << std::endl;
#endif

    return 0;
}
