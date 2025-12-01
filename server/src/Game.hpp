#pragma once

#include "UDPServer.hpp"
#include "ecs/components.hpp"
#include "ecs/entity.hpp"
#include "ecs/registry.hpp"
#include "ecs/sparse_array.hpp"
#include "ecs/systems.hpp"

#include <atomic>
#include <optional>
#include <unordered_map>

class Game {
private:
    registry _registry;
    std::unordered_map<int, std::size_t> _client_entity_ids;

    float _pos_broadcast_accumulator = 0.0f;
    float _cleanup_accumulator = 0.0f;

    void process_network_events(UDPServer& server);
    void update_game_state(float dt);
    void send_periodic_updates(UDPServer& server, float dt);

public:
    Game();
    ~Game();
    void runGameLoop(UDPServer& server);
    registry& getRegistry() { return _registry; }
    entity create_player(int client_id, float start_x = 100.0f, float start_y = 100.0f);
    std::optional<entity> get_player_entity(int client_id);
    void remove_player(int client_id);
    void broadcast_player_positions(UDPServer& server);
};