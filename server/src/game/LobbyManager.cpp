#include "game/LobbyManager.hpp"

#include "../../src/Common/CompressionSerializer.hpp"
#include "../../src/Common/Opcodes.hpp"

#include <algorithm>
#include <iostream>

namespace server {

LobbyManager::LobbyManager(int default_max_players)
    : _next_lobby_id(1), _default_max_players(default_max_players) {
    std::cout << "[LobbyManager] Initialized (default max players: "
              << default_max_players << ")" << std::endl;
}

int LobbyManager::create_lobby(const std::string& name, int max_players) {
    std::lock_guard<std::mutex> lock(_lobbies_mutex);

    int lobby_id = _next_lobby_id++;
    int actual_max = (max_players > 0) ? max_players : _default_max_players;

    auto lobby = std::make_unique<Lobby>(lobby_id, name, actual_max);
    _lobbies[lobby_id] = std::move(lobby);

    std::cout << "[LobbyManager] Created lobby " << lobby_id << ": " << name << std::endl;

    return lobby_id;
}

bool LobbyManager::delete_lobby(int lobby_id) {
    std::lock_guard<std::mutex> lock(_lobbies_mutex);

    auto it = _lobbies.find(lobby_id);
    if (it == _lobbies.end()) {
        return false;
    }

    std::cout << "[LobbyManager] Deleting lobby " << lobby_id << std::endl;

    {
        std::lock_guard<std::mutex> client_lock(_client_mapping_mutex);
        for (auto client_it = _client_to_lobby.begin(); client_it != _client_to_lobby.end();) {
            if (client_it->second == lobby_id) {
                client_it = _client_to_lobby.erase(client_it);
            } else {
                ++client_it;
            }
        }
    }

    _lobbies.erase(it);
    return true;
}

Lobby* LobbyManager::get_lobby(int lobby_id) {
    std::lock_guard<std::mutex> lock(_lobbies_mutex);

    auto it = _lobbies.find(lobby_id);
    if (it == _lobbies.end()) {
        return nullptr;
    }

    return it->second.get();
}

std::vector<LobbyInfo> LobbyManager::get_lobby_list() {
    std::lock_guard<std::mutex> lock(_lobbies_mutex);

    std::vector<LobbyInfo> list;
    for (const auto& [id, lobby] : _lobbies) {
        if (lobby->get_state() == LobbyState::Finished) {
            continue;
        }

        LobbyInfo info;
        info.lobby_id = lobby->get_id();
        info.name = lobby->get_name();
        info.current_players = lobby->get_player_count();
        info.max_players = lobby->get_max_players();
        info.state = lobby->get_state();
        list.push_back(info);
    }

    return list;
}

bool LobbyManager::join_lobby(int lobby_id, int client_id, UDPServer& server) {
    leave_lobby(client_id, server);

    Lobby* lobby = get_lobby(lobby_id);
    if (!lobby) {
        std::cout << "[LobbyManager] Cannot join lobby " << lobby_id
                  << ": lobby not found" << std::endl;
        return false;
    }

    if (lobby->add_player(client_id, server)) {
        std::lock_guard<std::mutex> lock(_client_mapping_mutex);
        _client_to_lobby[client_id] = lobby_id;
        return true;
    }

    return false;
}

bool LobbyManager::leave_lobby(int client_id, UDPServer& server) {
    int lobby_id;
    {
        std::lock_guard<std::mutex> lock(_client_mapping_mutex);
        auto it = _client_to_lobby.find(client_id);
        if (it == _client_to_lobby.end()) {
            return false;
        }
        lobby_id = it->second;
        _client_to_lobby.erase(it);
    }

    Lobby* lobby = get_lobby(lobby_id);
    if (lobby) {
        lobby->remove_player(client_id, server);
    }

    return true;
}

int LobbyManager::get_client_lobby(int client_id) {
    std::lock_guard<std::mutex> lock(_client_mapping_mutex);

    auto it = _client_to_lobby.find(client_id);
    if (it == _client_to_lobby.end()) {
        return -1;
    }

    return it->second;
}

Lobby* LobbyManager::get_client_lobby_ptr(int client_id) {
    int lobby_id = get_client_lobby(client_id);
    if (lobby_id < 0) {
        return nullptr;
    }

    return get_lobby(lobby_id);
}

void LobbyManager::update_all_lobbies(UDPServer& server, float dt) {
    std::lock_guard<std::mutex> lock(_lobbies_mutex);

    for (auto& [id, lobby] : _lobbies) {
        if (lobby->get_state() == LobbyState::InGame) {
            lobby->run_game_tick(server, dt);
        }
    }
}

void LobbyManager::cleanup_empty_lobbies() {
    std::lock_guard<std::mutex> lock(_lobbies_mutex);

    for (auto it = _lobbies.begin(); it != _lobbies.end();) {
        if (it->second->is_empty() && it->second->get_state() != LobbyState::InGame) {
            std::cout << "[LobbyManager] Removing empty lobby " << it->first << std::endl;
            it = _lobbies.erase(it);
        } else {
            ++it;
        }
    }
}

void LobbyManager::cleanup_inactive_lobbies(std::chrono::seconds timeout) {
    std::lock_guard<std::mutex> lock(_lobbies_mutex);

    for (auto it = _lobbies.begin(); it != _lobbies.end();) {
        if (it->second->is_inactive(timeout) && it->second->get_state() != LobbyState::InGame) {
            std::cout << "[LobbyManager] Removing inactive lobby " << it->first << std::endl;
            it = _lobbies.erase(it);
        } else {
            ++it;
        }
    }
}

void LobbyManager::handle_client_disconnect(int client_id, UDPServer& server) {
    std::cout << "[LobbyManager] Handling disconnect for client " << client_id << std::endl;
    leave_lobby(client_id, server);
}

void LobbyManager::broadcast_lobby_list(UDPServer& server) {
    auto lobbies = get_lobby_list();

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
    server.send_to_all(serializer.data());
}

}  // namespace server
