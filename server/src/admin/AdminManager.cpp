#include "admin/AdminManager.hpp"

#include "game/LobbyManager.hpp"
#include "network/UDPServer.hpp"

#include <algorithm>
#include <iomanip>
#include <random>
#include <sstream>

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wstringop-overflow"
#endif

namespace server {

AdminManager::AdminManager(const std::string& password_hash)
    : _password_hash(password_hash), _server_start_time(std::chrono::steady_clock::now()) {
    std::cout << "[AdminManager] Initialized with authentication" << std::endl;
}

std::string AdminManager::hash_password(const std::string& password) {
    return password;
}

std::string AdminManager::generate_token() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);

    std::stringstream ss;
    for (int i = 0; i < 32; ++i) {
        ss << std::hex << dis(gen);
    }
    return ss.str();
}

bool AdminManager::authenticate(int client_id, const std::string& password) {
    std::lock_guard<std::mutex> lock(_sessions_mutex);

    std::string hashed = hash_password(password);

    if (hashed != _password_hash) {
        std::cout << "[AdminManager] Failed authentication attempt from client " << client_id
                  << std::endl;
        return false;
    }

    AdminSession session;
    session.client_id = client_id;
    session.token = generate_token();
    session.last_activity = std::chrono::steady_clock::now();
    session.authenticated = true;

    _sessions[client_id] = session;

    std::cout << "[AdminManager] Client " << client_id << " authenticated as admin" << std::endl;
    return true;
}

bool AdminManager::is_authenticated(int client_id) const {
    std::lock_guard<std::mutex> lock(_sessions_mutex);
    auto it = _sessions.find(client_id);
    return it != _sessions.end() && it->second.authenticated;
}

void AdminManager::logout(int client_id) {
    std::lock_guard<std::mutex> lock(_sessions_mutex);
    _sessions.erase(client_id);
    std::cout << "[AdminManager] Client " << client_id << " logged out" << std::endl;
}

void AdminManager::cleanup_inactive_sessions() {
    std::lock_guard<std::mutex> lock(_sessions_mutex);
    auto now = std::chrono::steady_clock::now();

    for (auto it = _sessions.begin(); it != _sessions.end();) {
        auto elapsed =
            std::chrono::duration_cast<std::chrono::minutes>(now - it->second.last_activity);
        if (elapsed.count() > 30) {
            std::cout << "[AdminManager] Session timeout for client " << it->first << std::endl;
            it = _sessions.erase(it);
        } else {
            ++it;
        }
    }
}

AdminCommand AdminManager::parse_command(const std::string& command_str) {
    AdminCommand cmd;
    std::istringstream iss(command_str);
    std::string word;
    std::vector<std::string> words;

    while (iss >> word) {
        words.push_back(word);
    }

    if (words.empty()) {
        cmd.type = AdminCommand::Type::Help;
        return cmd;
    }

    std::string command = words[0];
    std::transform(command.begin(), command.end(), command.begin(), ::tolower);

    if (command == "list-players" || command == "players") {
        cmd.type = AdminCommand::Type::ListPlayers;
    } else if (command == "kick") {
        cmd.type = AdminCommand::Type::KickPlayer;
        if (words.size() > 1) {
            cmd.args.assign(words.begin() + 1, words.end());
        }
    } else if (command == "list-lobbies" || command == "lobbies") {
        cmd.type = AdminCommand::Type::ListLobbies;
    } else if (command == "close-lobby") {
        cmd.type = AdminCommand::Type::CloseLobby;
        if (words.size() > 1) {
            cmd.args.assign(words.begin() + 1, words.end());
        }
    } else if (command == "status") {
        cmd.type = AdminCommand::Type::ServerStatus;
    } else if (command == "announce") {
        cmd.type = AdminCommand::Type::Announce;
        if (words.size() > 1) {
            cmd.args.assign(words.begin() + 1, words.end());
        }
    } else {
        cmd.type = AdminCommand::Type::Help;
    }

    return cmd;
}

std::string AdminManager::execute_command(int client_id, const std::string& command_str,
                                          UDPServer& server, LobbyManager& lobby_manager) {
    if (!is_authenticated(client_id)) {
        return "ERROR: Not authenticated";
    }

    {
        std::lock_guard<std::mutex> lock(_sessions_mutex);
        auto it = _sessions.find(client_id);
        if (it != _sessions.end()) {
            it->second.last_activity = std::chrono::steady_clock::now();
        }
    }

    AdminCommand cmd = parse_command(command_str);

    switch (cmd.type) {
        case AdminCommand::Type::ListPlayers:
            return execute_list_players(server);
        case AdminCommand::Type::KickPlayer:
            return execute_kick_player(cmd.args, server, lobby_manager);
        case AdminCommand::Type::ListLobbies:
            return execute_list_lobbies(lobby_manager);
        case AdminCommand::Type::CloseLobby:
            return execute_close_lobby(cmd.args, lobby_manager);
        case AdminCommand::Type::ServerStatus:
            return execute_server_status(server, lobby_manager);
        case AdminCommand::Type::Announce:
            return execute_announce(cmd.args, server);
        case AdminCommand::Type::Help:
            return execute_help();
        default:
            return "ERROR: Unknown command. Type 'help' for available commands.";
    }
}

std::string AdminManager::execute_list_players(UDPServer& server) {
    auto clients = server.get_all_clients();

    std::stringstream ss;
    ss << "PLAYERS|" << clients.size() << "|";

    for (const auto& [client_id, endpoint] : clients) {
        ss << client_id << ";" << endpoint.address().to_string() << ";" << endpoint.port() << "|";
    }

    return ss.str();
}

std::string AdminManager::execute_kick_player(const std::vector<std::string>& args,
                                              UDPServer& server, LobbyManager& lobby_manager) {
    if (args.empty()) {
        return "ERROR: Usage: kick <client_id>";
    }

    try {
        int client_id = std::stoi(args[0]);

        std::cout << "[AdminManager] Removing player " << client_id << " from lobby" << std::endl;
        lobby_manager.handle_client_disconnect(client_id, server);

        lobby_manager.cleanup_empty_lobbies();

        std::cout << "[AdminManager] Disconnecting client " << client_id << std::endl;
        server.disconnect_client(client_id);

        return "OK: Player " + std::to_string(client_id) + " kicked";
    } catch (...) {
        return "ERROR: Invalid client ID";
    }
}

std::string AdminManager::execute_list_lobbies(LobbyManager& lobby_manager) {
    auto lobbies = lobby_manager.get_lobby_list();

    std::stringstream ss;
    ss << "LOBBIES|" << lobbies.size() << "|";

    for (const auto& lobby : lobbies) {
        ss << lobby.lobby_id << ";" << lobby.name << ";" << lobby.current_players << ";"
           << lobby.max_players << ";" << static_cast<int>(lobby.state) << "|";
    }

    return ss.str();
}

std::string AdminManager::execute_close_lobby(const std::vector<std::string>& args,
                                              LobbyManager& lobby_manager) {
    if (args.empty()) {
        return "ERROR: Usage: close-lobby <lobby_id>";
    }

    try {
        int lobby_id = std::stoi(args[0]);
        bool success = lobby_manager.delete_lobby(lobby_id);
        if (success) {
            return "OK: Lobby " + std::to_string(lobby_id) + " closed";
        } else {
            return "ERROR: Lobby not found";
        }
    } catch (...) {
        return "ERROR: Invalid lobby ID";
    }
}

std::string AdminManager::execute_server_status(UDPServer& server, LobbyManager& lobby_manager) {
    auto now = std::chrono::steady_clock::now();
    auto uptime = std::chrono::duration_cast<std::chrono::seconds>(now - _server_start_time);

    int hours = static_cast<int>(uptime.count() / 3600);
    int minutes = static_cast<int>((uptime.count() % 3600) / 60);
    int seconds = static_cast<int>(uptime.count() % 60);

    auto clients = server.get_all_clients();
    auto lobbies = lobby_manager.get_lobby_list();

    std::stringstream ss;
    ss << "STATUS|" << hours << "h " << minutes << "m " << seconds << "s|" << clients.size() << "|"
       << lobbies.size();

    return ss.str();
}

std::string AdminManager::execute_announce(const std::vector<std::string>& args,
                                           UDPServer& server) {
    (void)server;
    if (args.empty()) {
        return "ERROR: Usage: announce <message>";
    }

    std::string message;
    for (size_t i = 0; i < args.size(); ++i) {
        if (i > 0)
            message += " ";
        message += args[i];
    }

    std::cout << "[AdminManager] Announcement: " << message << std::endl;

    return "OK: Announcement sent";
}

std::string AdminManager::execute_help() {
    std::stringstream ss;
    ss << "HELP|"
       << "list-players - List all connected players|"
       << "kick <id> - Kick a player|"
       << "list-lobbies - Show all active lobbies|"
       << "close-lobby <id> - Close a lobby|"
       << "status - Show server status|"
       << "announce <message> - Send announcement|"
       << "help - Show this help";

    return ss.str();
}

}  // namespace server

#if defined(__GNUC__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif
