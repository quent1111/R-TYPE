#pragma once

#include "../../engine/ecs/components.hpp"
#include "../../engine/ecs/entity.hpp"
#include "../../engine/ecs/registry.hpp"
#include "../../engine/ecs/sparse_array.hpp"
#include "../../game-lib/include/components/game_components.hpp"
#include "../../game-lib/include/components/logic_components.hpp"
#include "../../game-lib/include/entities/enemy_factory.hpp"
#include "../../game-lib/include/entities/player_factory.hpp"
#include "../../game-lib/include/entities/projectile_factory.hpp"
#include "../../game-lib/include/systems/cleanup_system.hpp"
#include "../../game-lib/include/systems/collision_system.hpp"
#include "../../game-lib/include/systems/movement_system.hpp"
#include "../../game-lib/include/systems/shooting_system.hpp"
#include "../../game-lib/include/systems/wave_system.hpp"
#include "../../src/Common/BinarySerializer.hpp"
#include "UDPServer.hpp"

#include <atomic>
#include <optional>
#include <unordered_map>

enum class GamePhase { Lobby, InGame };

class Game {
private:
    registry _registry;
    std::unordered_map<int, std::size_t> _client_entity_ids;
    std::unordered_map<int, bool> _client_ready_status;

    RType::BinarySerializer _broadcast_serializer;

    float _pos_broadcast_accumulator = 0.0f;
    float _cleanup_accumulator = 0.0f;
    float _lobby_broadcast_accumulator = 0.0f;
    float _level_broadcast_accumulator = 0.0f;
    float _level_complete_timer = 0.0f;
    float _powerup_broadcast_accumulator = 0.0f;
    bool _level_complete_waiting = false;
    bool _waiting_for_powerup_choice = false;
    GamePhase _game_phase = GamePhase::Lobby;

    void process_network_events(UDPServer& server);
    void update_game_state(float dt);
    void send_periodic_updates(UDPServer& server, float dt);

    void handle_player_input(int client_id, const std::vector<uint8_t>& data);
    void handle_player_ready(int client_id, bool ready);
    void handle_weapon_upgrade_choice(int client_id, uint8_t upgrade_choice, UDPServer& server);
    void handle_powerup_choice(int client_id, uint8_t powerup_choice, UDPServer& server);
    void handle_powerup_activate(int client_id, UDPServer& server);
    void check_start_game(UDPServer& server);
    void start_game(UDPServer& server);
    void check_level_completion(UDPServer& server);
    void broadcast_level_info(UDPServer& server);
    void broadcast_level_complete(UDPServer& server);
    void broadcast_powerup_selection(UDPServer& server);
    void broadcast_powerup_status(UDPServer& server);
    void advance_level(UDPServer& server);

public:
    Game();
    ~Game();
    void runGameLoop(UDPServer& server);
    registry& getRegistry() { return _registry; }
    entity create_player(int client_id, float start_x = 100.0f, float start_y = 100.0f);
    std::optional<entity> get_player_entity(int client_id);
    void remove_player(int client_id);
    void broadcast_entity_positions(UDPServer& server);
    void broadcast_lobby_status(UDPServer& server);
};
