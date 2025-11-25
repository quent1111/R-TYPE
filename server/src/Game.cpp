#include "Game.hpp"
#include <iostream>
#include <thread>
#include <chrono>

extern std::atomic<bool> server_running;

Game::Game(/* args */)
{
}

Game::~Game()
{
}

void Game::runGameLoop(UDPServer& server) {
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
