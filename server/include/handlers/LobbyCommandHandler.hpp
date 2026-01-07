#pragma once

#include "../../src/Common/BinarySerializer.hpp"
#include "../../src/Common/Opcodes.hpp"
#include "game/LobbyManager.hpp"
#include "network/UDPServer.hpp"

#include <string>
#include <vector>

namespace server {

class LobbyCommandHandler {
private:
    LobbyManager& _lobby_manager;

public:
    explicit LobbyCommandHandler(LobbyManager& lobby_manager);
    ~LobbyCommandHandler() = default;

    void handle_list_lobbies(UDPServer& server, int client_id);
    void handle_create_lobby(UDPServer& server, int client_id, const std::vector<uint8_t>& data);
    void handle_join_lobby(UDPServer& server, int client_id, const std::vector<uint8_t>& data);
    void handle_leave_lobby(UDPServer& server, int client_id);
    void handle_start_game(UDPServer& server, int client_id);

private:
    void send_lobby_joined_ack(UDPServer& server, int client_id, int lobby_id, bool success);
    void send_lobby_left_ack(UDPServer& server, int client_id, bool success);
};

}  // namespace server
