#include "NetworkPacket.hpp"
#include "UDPServer.hpp"

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
    std::cout << "[Core] Game loop started at 60 TPS" << std::endl;

    const double target_ticks_per_second = 60.0;
    const std::chrono::duration<double> target_tick_duration(1.0 / target_ticks_per_second);
    auto last_broadcast = std::chrono::steady_clock::now();
    auto last_cleanup = std::chrono::steady_clock::now();

    auto frame_start_time = std::chrono::steady_clock::now();

    while (server_running) {
        frame_start_time = std::chrono::steady_clock::now();
        NetworkPacket packet;
        while (server.get_input_packet(packet)) {
            if (!packet.data.empty()) {
                NetworkPacket response_packet(packet.data, packet.sender);
                server.queue_output_packet(response_packet);
            }
        }

        if (frame_start_time - last_broadcast > std::chrono::seconds(10)) {
            std::string msg = "[GAME] Broadcast message (10s)";
            std::vector<uint8_t> broadcast_data(msg.begin(), msg.end());
            server.send_to_all(broadcast_data);

            std::cout << "[GAME] Broadcast sent." << std::endl;
            last_broadcast = frame_start_time;
        }

        if (frame_start_time - last_cleanup > std::chrono::seconds(30)) {
            server.remove_inactive_clients(std::chrono::seconds(30));
            std::cout << "[GAME] Cleanup performed." << std::endl;
            last_cleanup = frame_start_time;
        }

        auto execution_time = std::chrono::steady_clock::now() - frame_start_time;
        if (execution_time < target_tick_duration) {
            auto time_to_sleep = target_tick_duration - execution_time;
            std::this_thread::sleep_for(time_to_sleep);
        }
    }
    std::cout << "[Core] Game loop stopped" << std::endl;
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
        //Declarer les fonctions du bootstrap ici
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
