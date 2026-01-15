#include "handlers/LobbyCommandHandler.hpp"

#include <iostream>
#include "../../src/Common/CompressionSerializer.hpp"
#include "../../src/Common/Opcodes.hpp"

namespace server {

LobbyCommandHandler::LobbyCommandHandler(LobbyManager& lobby_manager)
    : _lobby_manager(lobby_manager) {
    std::cout << "[LobbyCommandHandler] Initialized" << std::endl;
}

void LobbyCommandHandler::handle_list_lobbies(UDPServer& server, int client_id) {
    std::cout << "[LobbyCommandHandler] Client " << client_id << " requested lobby list" << std::endl;

    auto lobbies = _lobby_manager.get_lobby_list();

    RType::CompressionSerializer serializer;
    serializer << RType::MagicNumber::VALUE;
    serializer << static_cast<uint8_t>(RType::OpCode::ListLobbies);
    serializer << static_cast<int32_t>(lobbies.size());

    for (const auto& lobby : lobbies) {
        serializer << static_cast<int32_t>(lobby.lobby_id);
        serializer << lobby.name;
        serializer << static_cast<int32_t>(lobby.current_players);
        serializer << static_cast<int32_t>(lobby.max_players);
        serializer << static_cast<uint8_t>(lobby.state);
    }

    serializer.compress();
    std::cout << "[LobbyCommandHandler] Sending " << lobbies.size() << " lobbies to client " << client_id << std::endl;
    server.send_to_client(client_id, serializer.data());
}

void LobbyCommandHandler::handle_create_lobby(UDPServer& server, int client_id,
                                               const std::vector<uint8_t>& data) {
    RType::BinarySerializer deserializer(data);

    uint16_t magic;
    uint8_t opcode_val;
    deserializer >> magic >> opcode_val;

    std::string lobby_name;
    bool friendly_fire = false;
    uint8_t difficulty = 0;
    deserializer >> lobby_name;
    deserializer >> friendly_fire;
    deserializer >> difficulty;

    if (lobby_name.length() > 12) {
        std::cerr << "[LobbyCommandHandler] Lobby name too long (" << lobby_name.length()
                  << "), truncating to 12 characters" << std::endl;
        lobby_name = lobby_name.substr(0, 12);
    }

    uint8_t max_players = 4;

    std::cout << "[LobbyCommandHandler] Client " << client_id << " creating lobby: " 
              << lobby_name << " (max " << static_cast<int>(max_players) << " players)"
              << " Friendly Fire: " << (friendly_fire ? "ON" : "OFF")
              << " Difficulty: " << static_cast<int>(difficulty) << std::endl;

    int lobby_id = _lobby_manager.create_lobby(lobby_name, max_players, friendly_fire, difficulty);

    bool joined = _lobby_manager.join_lobby(lobby_id, client_id, server);

    send_lobby_joined_ack(server, client_id, lobby_id, joined);

    _lobby_manager.broadcast_lobby_list(server);
}

void LobbyCommandHandler::handle_join_lobby(UDPServer& server, int client_id,
                                            const std::vector<uint8_t>& data) {
    RType::BinarySerializer deserializer(data);

    uint16_t magic;
    uint8_t opcode_val;
    deserializer >> magic >> opcode_val;

    int32_t lobby_id;
    deserializer >> lobby_id;

    std::cout << "[LobbyCommandHandler] Client " << client_id << " joining lobby " 
              << lobby_id << std::endl;

    bool success = _lobby_manager.join_lobby(lobby_id, client_id, server);

    send_lobby_joined_ack(server, client_id, lobby_id, success);

    if (success) {
        Lobby* lobby = _lobby_manager.get_lobby(lobby_id);
        if (lobby && lobby->get_game_session()) {
            lobby->get_game_session()->broadcast_lobby_status(server);
        }
        _lobby_manager.broadcast_lobby_list(server);
    }
}

void LobbyCommandHandler::handle_leave_lobby(UDPServer& server, int client_id) {
    std::cout << "[LobbyCommandHandler] Client " << client_id << " leaving lobby" << std::endl;

    bool success = _lobby_manager.leave_lobby(client_id, server);

    send_lobby_left_ack(server, client_id, success);

    if (success) {
        _lobby_manager.broadcast_lobby_list(server);
    }
}

void LobbyCommandHandler::handle_start_game(UDPServer& server, int client_id) {
    std::cout << "[LobbyCommandHandler] Client " << client_id << " requesting game start" << std::endl;

    Lobby* lobby = _lobby_manager.get_client_lobby_ptr(client_id);
    if (!lobby) {
        std::cerr << "[LobbyCommandHandler] Client " << client_id << " not in any lobby" << std::endl;
        return;
    }

    lobby->start_game(server);

    _lobby_manager.broadcast_lobby_list(server);
}

void LobbyCommandHandler::send_lobby_joined_ack(UDPServer& server, int client_id, int lobby_id,
                                                 bool success) {
    RType::CompressionSerializer serializer;
    serializer << RType::MagicNumber::VALUE;
    serializer << static_cast<uint8_t>(RType::OpCode::LobbyJoined);
    serializer << static_cast<uint8_t>(success ? 1 : 0);
    serializer << static_cast<int32_t>(lobby_id);
    serializer.compress();

    server.send_to_client(client_id, serializer.data());

    std::cout << "[LobbyCommandHandler] Sent LobbyJoined ack to client " << client_id 
              << " (success=" << success << ", lobby=" << lobby_id << ")" << std::endl;
}

void LobbyCommandHandler::send_lobby_left_ack(UDPServer& server, int client_id, bool success) {
    RType::CompressionSerializer serializer;
    serializer << RType::MagicNumber::VALUE;
    serializer << static_cast<uint8_t>(RType::OpCode::LobbyLeft);
    serializer << static_cast<uint8_t>(success ? 1 : 0);
    serializer.compress();

    server.send_to_client(client_id, serializer.data());

    std::cout << "[LobbyCommandHandler] Sent LobbyLeft ack to client " << client_id 
              << " (success=" << success << ")" << std::endl;
}

}  // namespace server
