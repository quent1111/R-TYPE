#include "game/ServerCore.hpp"

#include "../../src/Common/CompressionSerializer.hpp"
#include "../../src/Common/Opcodes.hpp"
#include "common/NetworkPacket.hpp"
#include "common/EnvLoader.hpp"

#include <chrono>
#include <iostream>
#include <thread>

extern std::atomic<bool> server_running;

namespace server {

ServerCore::ServerCore()
    : _lobby_manager(4),
      _lobby_command_handler(_lobby_manager),
      _admin_manager(nullptr) {
    auto env_vars = EnvLoader::load(".env");
    std::string admin_password = EnvLoader::get(env_vars, "ADMIN_PASSWORD", "admin123");

    _admin_manager = std::make_unique<AdminManager>(admin_password);

    std::cout << "[ServerCore] Initialized" << std::endl;
    std::cout << "[ServerCore] Admin system enabled" << std::endl;
}

void ServerCore::process_network_events(UDPServer& server) {
    NetworkPacket packet;
    while (server.get_input_packet(packet)) {
        if (packet.data.empty() || packet.data.size() < 3) {
            continue;
        }

        try {
            RType::BinarySerializer deserializer(packet.data);

            uint16_t magic;
            deserializer >> magic;

            if (!RType::MagicNumber::is_valid(magic)) {
                std::cerr << "[ServerCore] Invalid magic number from " << packet.sender << std::endl;
                continue;
            }

            RType::OpCode opcode;
            deserializer >> opcode;
            int client_id = server.register_client(packet.sender);

            switch (opcode) {
                case RType::OpCode::Login: {
                    std::cout << "[ServerCore] Login request from client " << client_id << std::endl;
                    RType::CompressionSerializer ack_serializer;
                    ack_serializer << RType::MagicNumber::VALUE;
                    ack_serializer << static_cast<uint8_t>(RType::OpCode::LoginAck);
                    ack_serializer << static_cast<uint32_t>(client_id);
                    ack_serializer.compress();
                    server.send_to_client(client_id, ack_serializer.data());
                    continue;
                }
                case RType::OpCode::Keepalive:
                    continue;
                case RType::OpCode::ListLobbies:
                    _lobby_command_handler.handle_list_lobbies(server, client_id);
                    continue;
                case RType::OpCode::CreateLobby:
                    _lobby_command_handler.handle_create_lobby(server, client_id, packet.data);
                    continue;
                case RType::OpCode::JoinLobby:
                    _lobby_command_handler.handle_join_lobby(server, client_id, packet.data);
                    continue;
                case RType::OpCode::LeaveLobby:
                    _lobby_command_handler.handle_leave_lobby(server, client_id);
                    continue;
                case RType::OpCode::StartGame:
                    _lobby_command_handler.handle_start_game(server, client_id);
                    continue;
                case RType::OpCode::AdminLogin: {
                    std::string password;
                    deserializer >> password;

                    bool success = _admin_manager->authenticate(client_id, password);

                    RType::BinarySerializer response;
                    response << RType::MagicNumber::VALUE;
                    response << RType::OpCode::AdminLoginAck;
                    response << (success ? std::string("OK: Authenticated") : std::string("ERROR: Authentication failed"));
                    server.send_to_client(client_id, response.data());
                    std::cout << "[ServerCore] Admin login attempt from client " << client_id 
                              << ": " << (success ? "SUCCESS" : "FAILED") << std::endl;
                    continue;
                }
                case RType::OpCode::AdminCommand: {
                    std::string command;
                    deserializer >> command;

                    std::string result = _admin_manager->execute_command(client_id, command, server, _lobby_manager);

                    RType::BinarySerializer response;
                    response << RType::MagicNumber::VALUE;
                    response << RType::OpCode::AdminResponse;
                    response << result;

                    server.send_to_client(client_id, response.data());
                    continue;
                }
                case RType::OpCode::AdminLogout: {
                    _admin_manager->logout(client_id);
                    std::cout << "[ServerCore] Admin client " << client_id << " logged out" << std::endl;
                    continue;
                }
                default:
                    break;
            }

            Lobby* client_lobby = _lobby_manager.get_client_lobby_ptr(client_id);
            if (!client_lobby) {
                std::cerr << "[ServerCore] Client " << client_id
                          << " not in a lobby, ignoring opcode "
                          << RType::opcode_to_string(opcode) << std::endl;
                continue;
            }

            GameSession* game_session = client_lobby->get_game_session();
            if (!game_session) {
                std::cerr << "[ServerCore] Lobby has no game session" << std::endl;
                continue;
            }

            switch (opcode) {
                case RType::OpCode::Input:
                case RType::OpCode::PlayerReady:
                case RType::OpCode::WeaponUpgradeChoice:
                case RType::OpCode::PowerUpChoice:
                case RType::OpCode::PowerUpActivate:
                    game_session->handle_packet(server, client_id, packet.data);
                    break;
                default:
                    std::cout << "[ServerCore] Unknown opcode from client " << client_id
                              << ": " << RType::opcode_to_string(opcode) << std::endl;
                    break;
            }

        } catch (const std::exception& e) {
            std::cerr << "[ServerCore] Exception processing packet: " << e.what() << std::endl;
        }
    }
}

void ServerCore::update_lobbies(UDPServer& server, float dt) {
    _lobby_manager.update_all_lobbies(server, dt);
}

void ServerCore::periodic_cleanup(UDPServer& server, float dt) {
    _cleanup_accumulator += dt;
    _broadcast_accumulator += dt;
    if (_cleanup_accumulator >= 5.0f) {
        _cleanup_accumulator = 0.0f;

        auto inactive_clients = server.remove_inactive_clients(std::chrono::seconds(300));
        for (int client_id : inactive_clients) {
            std::cout << "[ServerCore] Removing inactive client " << client_id << std::endl;
            _lobby_manager.handle_client_disconnect(client_id, server);
        }
        _lobby_manager.cleanup_inactive_lobbies(std::chrono::seconds(300)); // 5 minutes
    }
    if (_broadcast_accumulator >= 2.0f) {
        _broadcast_accumulator = 0.0f;
        _lobby_manager.broadcast_lobby_list(server);
    }
}

void ServerCore::run_game_loop(UDPServer& server) {
    std::cout << "[ServerCore] Game loop started" << std::endl;

    auto last_time = std::chrono::steady_clock::now();
    const float target_dt = 1.0f / 60.0f;  // 60 FPS

    while (server_running) {
        auto current_time = std::chrono::steady_clock::now();
        float dt = std::chrono::duration<float>(current_time - last_time).count();
        last_time = current_time;
        if (dt > 0.1f) {
            dt = target_dt;
        }
        process_network_events(server);
        update_lobbies(server, dt);
        periodic_cleanup(server, dt);
        auto frame_time = std::chrono::steady_clock::now() - current_time;
        auto target_frame_time = std::chrono::duration<float>(target_dt);
        if (frame_time < target_frame_time) {
            std::this_thread::sleep_for(target_frame_time - frame_time);
        }
    }

    std::cout << "[ServerCore] Game loop stopped" << std::endl;
}

}  // namespace server
