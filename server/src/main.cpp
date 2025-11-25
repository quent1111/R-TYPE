#include "UDPServer.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
#include <csignal>
#include "NetworkPacket.hpp"

std::atomic<bool> server_running{true};

void signal_handler(int signal) {
    if (signal == SIGINT || signal == SIGTERM) {
        std::cout << "\nShutting down server..." << std::endl;
        server_running = false;
    }
}

void game_loop(UDPServer& server) {
    std::cout << "Game loop started" << std::endl;

    auto last_update = std::chrono::steady_clock::now();
    const std::chrono::milliseconds tick_duration(16); // FPS(60)

    while (server_running) {
        auto now = std::chrono::steady_clock::now();
        auto delta_time = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_update);

        if (delta_time >= tick_duration) {
            last_update = now;

            NetworkPacket packet;
            while (server.get_input_packet(packet)) {
                std::cout << "[GAME] Processing packet from "
                          << packet.sender.address().to_string()
                          << ": " << packet.data.size() << " bytes" << std::endl;

                if (!packet.data.empty()) {
                    std::vector<uint8_t> response = packet.data;
                    std::cout << packet.sender.address().to_string() << ":" << packet.sender.port() << std::endl;
                    NetworkPacket response_packet(response, packet.sender);
                    server.queue_output_packet(response_packet);
                }
            }
            static int counter = 0;

            //Envoyer un message a tout le monde
            if (++counter % 600 == 0) {
                std::string msg = "[GAME] Message send to all clients";
                std::vector<uint8_t> broadcast_data(msg.begin(), msg.end());
                server.send_to_all(broadcast_data);
                std::cout << "[GAME] Sent broadcast to "
                          << server.get_client_count() << " clients" << std::endl;
            }

            // Enlever les clients inactifs toutes les 50 secondes
            if (counter % 3000 == 0) {
                server.remove_inactive_clients(std::chrono::seconds(30));
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    std::cout << "Game loop stopped" << std::endl;
}

void network_loop(UDPServer& server) {
    std::cout << "Network loop started" << std::endl;
    server.run_network_loop();
    std::cout << "Network loop stopped" << std::endl;
}

int main(int argc, char** argv) {
    std::signal(SIGINT, signal_handler);
    std::signal(SIGTERM, signal_handler);

    unsigned short port = 4242;
    if (argc > 1) {
        port = static_cast<unsigned short>(std::stoi(argv[1]));
    }

    std::cout << "R-Type Server Starting" << std::endl;
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
