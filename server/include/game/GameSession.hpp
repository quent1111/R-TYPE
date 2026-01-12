#pragma once

#include "../../engine/core/GameEngine.hpp"
#include "../../engine/ecs/components.hpp"
#include "../../engine/ecs/entity.hpp"
#include "../../engine/ecs/registry.hpp"
#include "../../engine/ecs/sparse_array.hpp"
#include "../../game-lib/include/components/game_components.hpp"
#include "../../game-lib/include/components/logic_components.hpp"
#include "../../game-lib/include/entities/boss_factory.hpp"
#include "../../game-lib/include/entities/enemy_factory.hpp"
#include "../../game-lib/include/entities/player_factory.hpp"
#include "../../game-lib/include/entities/projectile_factory.hpp"
#include "../../game-lib/include/systems/system_wrappers.hpp"
#include "../../src/Common/CompressionSerializer.hpp"
#include "../../src/Common/Opcodes.hpp"
#include "common/GameConstants.hpp"
#include "game/BossManager.hpp"
#include "game/LevelManager.hpp"
#include "game/PlayerManager.hpp"
#include "handlers/InputHandler.hpp"
#include "handlers/PowerupHandler.hpp"
#include "handlers/WeaponHandler.hpp"
#include "network/EntityBroadcaster.hpp"
#include "network/GameBroadcaster.hpp"
#include "network/LobbyBroadcaster.hpp"
#include "network/PowerupBroadcaster.hpp"
#include "network/UDPServer.hpp"

#include <atomic>
#include <optional>
#include <set>
#include <unordered_map>
#include <unordered_set>

namespace server {

class GameSession {
private:
    engine::GameEngine _engine;
    std::unordered_map<int, std::size_t> _client_entity_ids;
    std::unordered_map<int, bool> _client_ready_status;
    std::vector<int> _lobby_client_ids;
    std::unordered_map<int, int> _client_player_index;
    std::set<int> _available_player_slots;

    PlayerManager _player_manager;
    LevelManager _level_manager;
    BossManager _boss_manager;

    InputHandler _input_handler;
    PowerupHandler _powerup_handler;
    WeaponHandler _weapon_handler;

    EntityBroadcaster _entity_broadcaster;
    LobbyBroadcaster _lobby_broadcaster;
    GameBroadcaster _game_broadcaster;
    PowerupBroadcaster _powerup_broadcaster;

    float _pos_broadcast_accumulator = 0.0f;
    float _cleanup_accumulator = 0.0f;
    float _lobby_broadcast_accumulator = 0.0f;
    float _level_broadcast_accumulator = 0.0f;
    float _level_complete_timer = 0.0f;
    float _powerup_broadcast_accumulator = 0.0f;
    float _powerup_choice_timer = 0.0f;
    float _game_over_timer = 0.0f;
    float _game_over_broadcast_accumulator = 0.0f;
    bool _level_complete_waiting = false;
    bool _waiting_for_powerup_choice = false;
    bool _waiting_for_game_over_reset = false;
    std::unordered_set<int> _players_who_chose_powerup;
    GamePhase _game_phase = GamePhase::Lobby;

    std::optional<entity> _boss_entity;
    float _boss_animation_timer = 0.0f;
    float _boss_shoot_timer = 0.0f;
    float _boss_shoot_cooldown = 1.0f;
    bool _boss_animation_complete = false;
    bool _boss_entrance_complete = false;
    float _boss_target_x = 1500.0f;
    int _boss_shoot_counter = 0;

    std::optional<entity> _serpent_controller_entity;
    
    std::string _lobby_name;
    int _starting_level = 1;

    void process_network_events(UDPServer& server);
    void update_game_state(UDPServer& server, float dt);
    void send_periodic_updates(UDPServer& server, float dt);

    void check_level_completion(UDPServer& server);
    void advance_level(UDPServer& server);
    void reset_game(UDPServer& server);

    std::function<void()> _game_reset_callback = [](){};

public:
    void set_game_reset_callback(std::function<void()> callback) {
        _game_reset_callback = callback;
    }

    void handle_player_ready(int client_id, bool ready);
    void check_start_game(UDPServer& server);
    void start_game(UDPServer& server);
    void set_lobby_clients(const std::vector<int>& client_ids) { _lobby_client_ids = client_ids; }
    void set_lobby_name(const std::string& name);
    void broadcast_lobby_status(UDPServer& server);
    void remove_player(int client_id);
    void create_player_for_client(int client_id, float start_x, float start_y);
    void send_game_state_to_client(UDPServer& server, int client_id);
    void notify_game_started(UDPServer& server, int client_id);
    GameSession();
    ~GameSession();
    void runGameLoop(UDPServer& server);
    void update(UDPServer& server, float dt);
    void process_inputs(UDPServer& server);
    void handle_packet(UDPServer& server, int client_id, const std::vector<uint8_t>& data);
    registry& getRegistry() { return _engine.get_registry(); }
};

}  // namespace server
