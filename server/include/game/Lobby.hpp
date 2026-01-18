#pragma once

#include "GameSession.hpp"
#include "common/GameConstants.hpp"

#include <chrono>
#include <memory>
#include <string>
#include <vector>

namespace server {

enum class LobbyState { Waiting, Ready, InGame, Finished };

class Lobby {
private:
    int _lobby_id;
    std::string _lobby_name;
    std::unique_ptr<GameSession> _game_session;
    std::vector<int> _player_ids;
    int _max_players;
    LobbyState _state;
    bool _friendly_fire;
    uint8_t _difficulty;
    std::chrono::steady_clock::time_point _last_activity;

public:
    Lobby(int lobby_id, const std::string& name, int max_players = 4, bool friendly_fire = false,
          uint8_t difficulty = 0);
    ~Lobby() = default;

    int get_id() const { return _lobby_id; }
    const std::string& get_name() const { return _lobby_name; }
    int get_player_count() const { return static_cast<int>(_player_ids.size()); }
    int get_max_players() const { return _max_players; }
    LobbyState get_state() const { return _state; }
    bool get_friendly_fire() const { return _friendly_fire; }
    uint8_t get_difficulty() const { return _difficulty; }
    const std::vector<int>& get_player_ids() const { return _player_ids; }
    GameSession* get_game_session() { return _game_session.get(); }
    bool is_full() const { return _player_ids.size() >= static_cast<size_t>(_max_players); }
    bool is_empty() const { return _player_ids.empty(); }
    bool has_player(int client_id) const;

    bool add_player(int client_id, UDPServer& server);
    bool remove_player(int client_id, UDPServer& server);

    void set_state(LobbyState state) { _state = state; }
    void update_activity();
    bool is_inactive(std::chrono::seconds timeout) const;
    void reset_after_game_over();

    void start_game(UDPServer& server);
    void run_game_tick(UDPServer& server, float dt);
};

}  // namespace server
