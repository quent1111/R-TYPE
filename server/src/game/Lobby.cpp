#include "game/Lobby.hpp"

#include <algorithm>
#include <iostream>

namespace server {

Lobby::Lobby(int lobby_id, const std::string& name, int max_players, bool friendly_fire,
             uint8_t difficulty)
    : _lobby_id(lobby_id),
      _lobby_name(name),
      _game_session(std::make_unique<GameSession>()),
      _max_players(max_players),
      _state(LobbyState::Waiting),
      _friendly_fire(friendly_fire),
      _difficulty(difficulty),
      _last_activity(std::chrono::steady_clock::now()) {
    std::cout << "[Lobby " << _lobby_id << "] Created: " << _lobby_name << " (max " << _max_players
              << " players)"
              << " Friendly Fire: " << (_friendly_fire ? "ON" : "OFF")
              << " Difficulty: " << static_cast<int>(_difficulty) << std::endl;

    _game_session->set_game_reset_callback([this]() { this->reset_after_game_over(); });

    if (_game_session) {
        _game_session->set_lobby_name(_lobby_name);
        _game_session->set_difficulty(_difficulty);
    }
}

bool Lobby::has_player(int client_id) const {
    return std::find(_player_ids.begin(), _player_ids.end(), client_id) != _player_ids.end();
}

bool Lobby::add_player(int client_id, UDPServer& server) {
    if (is_full()) {
        std::cout << "[Lobby " << _lobby_id << "] Cannot add player " << client_id
                  << ": lobby is full" << std::endl;
        return false;
    }

    if (has_player(client_id)) {
        std::cout << "[Lobby " << _lobby_id << "] Player " << client_id << " already in lobby"
                  << std::endl;
        return false;
    }

    _player_ids.push_back(client_id);
    update_activity();

    if (_game_session) {
        _game_session->set_lobby_clients(_player_ids);

        if (_state == LobbyState::InGame) {
            std::cout << "[Lobby " << _lobby_id << "] Player " << client_id
                      << " joining game in progress" << std::endl;

            float start_x = 100.0f + (static_cast<float>(client_id) * 50.0f);
            float start_y = 300.0f;
            _game_session->create_player_for_client(client_id, start_x, start_y);

            _game_session->send_game_state_to_client(server, client_id);

            _game_session->notify_game_started(server, client_id);
        } else {
            _game_session->handle_player_ready(client_id, false);
            _game_session->broadcast_lobby_status(server);
        }
    }

    std::cout << "[Lobby " << _lobby_id << "] Player " << client_id << " joined ("
              << _player_ids.size() << "/" << _max_players << ")" << std::endl;

    return true;
}

bool Lobby::remove_player(int client_id, UDPServer& server) {
    auto it = std::find(_player_ids.begin(), _player_ids.end(), client_id);
    if (it == _player_ids.end()) {
        return false;
    }

    _player_ids.erase(it);
    update_activity();

    std::cout << "[Lobby " << _lobby_id << "] Player " << client_id << " left ("
              << _player_ids.size() << "/" << _max_players << ")" << std::endl;

    if (_game_session) {
        _game_session->set_lobby_clients(_player_ids);
        _game_session->remove_player(client_id);

        if (!_player_ids.empty()) {
            _game_session->broadcast_lobby_status(server);
        }
    }

    if (is_empty()) {
        _state = LobbyState::Finished;
        std::cout << "[Lobby " << _lobby_id << "] All players left, marking as finished"
                  << std::endl;
    }

    return true;
}

void Lobby::update_activity() {
    _last_activity = std::chrono::steady_clock::now();
}

bool Lobby::is_inactive(std::chrono::seconds timeout) const {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - _last_activity);
    return elapsed >= timeout;
}

void Lobby::reset_after_game_over() {
    std::cout << "[Lobby " << _lobby_id << "] Game over - marking lobby as finished" << std::endl;
    _state = LobbyState::Finished;
    update_activity();
}

void Lobby::start_game(UDPServer& server) {
    if (_state == LobbyState::InGame) {
        std::cout << "[Lobby " << _lobby_id << "] Game already started" << std::endl;
        return;
    }

    if (_player_ids.empty()) {
        std::cout << "[Lobby " << _lobby_id << "] Cannot start game: no players" << std::endl;
        return;
    }

    std::cout << "[Lobby " << _lobby_id << "] Starting game with " << _player_ids.size()
              << " players" << std::endl;

    _game_session->set_lobby_clients(_player_ids);
    _game_session->set_friendly_fire(_friendly_fire);

    for (int player_id : _player_ids) {
        _game_session->handle_player_ready(player_id, true);
    }

    _game_session->start_game(server);
    _state = LobbyState::InGame;
    update_activity();
}

void Lobby::run_game_tick(UDPServer& server, float dt) {
    if (_state == LobbyState::InGame && _game_session) {
        _game_session->process_inputs(server);
        _game_session->update(server, dt);
        update_activity();
    }
}

}  // namespace server
