#pragma once

#include "game/LobbyManager.hpp"
#include "handlers/LobbyCommandHandler.hpp"
#include "admin/AdminManager.hpp"
#include "network/UDPServer.hpp"

#include <atomic>
#include <memory>

namespace server {

class ServerCore {
private:
    LobbyManager _lobby_manager;
    LobbyCommandHandler _lobby_command_handler;
    std::unique_ptr<AdminManager> _admin_manager;

    float _cleanup_accumulator = 0.0f;
    float _broadcast_accumulator = 0.0f;

    void process_network_events(UDPServer& server);
    void update_lobbies(UDPServer& server, float dt);
    void periodic_cleanup(UDPServer& server, float dt);

public:
    ServerCore();
    ~ServerCore() = default;

    void run_game_loop(UDPServer& server);
    LobbyManager& get_lobby_manager() { return _lobby_manager; }
};

}  // namespace server
