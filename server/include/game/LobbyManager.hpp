#pragma once

#include "Lobby.hpp"
#include "network/UDPServer.hpp"

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace server {

struct LobbyInfo {
    int lobby_id;
    std::string name;
    int current_players;
    int max_players;
    LobbyState state;
};

class LobbyManager {
private:
    std::map<int, std::unique_ptr<Lobby>> _lobbies;
    std::mutex _lobbies_mutex;
    int _next_lobby_id;
    int _default_max_players;

    std::map<int, int> _client_to_lobby;
    std::mutex _client_mapping_mutex;

public:
    LobbyManager(int default_max_players = 4);
    ~LobbyManager() = default;

    int create_lobby(const std::string& name, int max_players = 0);
    bool delete_lobby(int lobby_id);
    Lobby* get_lobby(int lobby_id);
    std::vector<LobbyInfo> get_lobby_list();

    bool join_lobby(int lobby_id, int client_id, UDPServer& server);
    bool leave_lobby(int client_id, UDPServer& server);
    int get_client_lobby(int client_id);
    Lobby* get_client_lobby_ptr(int client_id);

    void update_all_lobbies(UDPServer& server, float dt);

    void cleanup_empty_lobbies();
    void cleanup_inactive_lobbies(std::chrono::seconds timeout);
    void handle_client_disconnect(int client_id, UDPServer& server);

    void broadcast_lobby_list(UDPServer& server);
};

}  // namespace server
