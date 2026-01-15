#pragma once

#include <chrono>
#include <mutex>
#include <string>
#include <unordered_map>
#include <vector>

namespace server {

class UDPServer;
class LobbyManager;

struct AdminSession {
    int client_id;
    std::string token;
    std::chrono::steady_clock::time_point last_activity;
    bool authenticated;
};

struct AdminCommand {
    enum class Type {
        ListPlayers,
        KickPlayer,
        ListLobbies,
        CloseLobby,
        ForceStart,
        ForceStop,
        ServerStatus,
        Announce,
        GetConfig,
        SetConfig,
        Shutdown,
        Help
    };

    Type type;
    std::vector<std::string> args;
};

class AdminManager {
public:
    AdminManager(const std::string& password_hash);
    ~AdminManager() = default;

    bool authenticate(int client_id, const std::string& password);
    bool is_authenticated(int client_id) const;
    void logout(int client_id);
    void cleanup_inactive_sessions();

    std::string execute_command(int client_id, const std::string& command_str, UDPServer& server,
                                LobbyManager& lobby_manager);

private:
    std::string _password_hash;
    std::unordered_map<int, AdminSession> _sessions;
    mutable std::mutex _sessions_mutex;

    std::chrono::steady_clock::time_point _server_start_time;

    AdminCommand parse_command(const std::string& command_str);
    std::string execute_list_players(UDPServer& server);
    std::string execute_kick_player(const std::vector<std::string>& args, UDPServer& server,
                                    LobbyManager& lobby_manager);
    std::string execute_list_lobbies(LobbyManager& lobby_manager);
    std::string execute_close_lobby(const std::vector<std::string>& args,
                                    LobbyManager& lobby_manager);
    std::string execute_server_status(UDPServer& server, LobbyManager& lobby_manager);
    std::string execute_announce(const std::vector<std::string>& args, UDPServer& server);
    std::string execute_help();

    std::string hash_password(const std::string& password);
    std::string generate_token();
};

}  // namespace server
