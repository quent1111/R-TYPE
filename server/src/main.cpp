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
    std::cout << "[Core] Game loop started at 60 TPS (fixed timestep)" << std::endl;

    const double target_ticks_per_second = 60.0;
    const std::chrono::duration<double> fixed_timestep(1.0 / target_ticks_per_second);
    const double dt = 1.0 / target_ticks_per_second;

    double broadcast_accumulator = 0.0;
    double cleanup_accumulator = 0.0;
    const double broadcast_interval = 10.0;
    const double cleanup_interval = 30.0;

    auto previous_time = std::chrono::steady_clock::now();
    std::chrono::duration<double> lag(0.0);

    while (server_running) {
        auto current_time = std::chrono::steady_clock::now();
        std::chrono::duration<double> elapsed = current_time - previous_time;
        previous_time = current_time;
        lag += elapsed;

        while (lag >= fixed_timestep) {
            NetworkPacket packet;
            while (server.get_input_packet(packet)) {
                if (!packet.data.empty()) {
                    NetworkPacket response_packet(packet.data, packet.sender);
                    server.queue_output_packet(std::move(response_packet));
                }
            }

            broadcast_accumulator += dt;
            if (broadcast_accumulator >= broadcast_interval) {
                std::string msg = "[GAME] Broadcast message (10s)";
                std::vector<uint8_t> broadcast_data(msg.begin(), msg.end());
                server.send_to_all(broadcast_data);
                std::cout << "[GAME] Broadcast sent to " << server.get_client_count()
                          << " client(s)" << std::endl;
                broadcast_accumulator -= broadcast_interval;
            }

            cleanup_accumulator += dt;
            if (cleanup_accumulator >= cleanup_interval) {
                server.remove_inactive_clients(std::chrono::seconds(30));
                std::cout << "[GAME] Cleanup performed. Active clients: "
                          << server.get_client_count() << std::endl;
                cleanup_accumulator -= cleanup_interval;
            }

            lag -= fixed_timestep;
        }

        auto frame_end = std::chrono::steady_clock::now();
        auto frame_duration = frame_end - current_time;
        if (frame_duration < fixed_timestep) {
            std::this_thread::sleep_for(fixed_timestep - frame_duration);
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
