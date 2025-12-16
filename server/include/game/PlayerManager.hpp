#pragma once

#include "../../engine/ecs/components.hpp"
#include "../../engine/ecs/entity.hpp"
#include "../../engine/ecs/registry.hpp"
#include "../../game-lib/include/components/game_components.hpp"
#include "../../game-lib/include/entities/player_factory.hpp"

#include <iostream>
#include <optional>
#include <unordered_map>
#include <vector>

namespace server {

class PlayerManager {
public:
    PlayerManager() = default;
    ~PlayerManager() = default;

    entity create_player(registry& reg, std::unordered_map<int, std::size_t>& client_entity_ids,
                         int client_id, float start_x = 100.0f, float start_y = 100.0f);

    std::optional<entity>
    get_player_entity(registry& reg, const std::unordered_map<int, std::size_t>& client_entity_ids,
                      int client_id);

    void remove_player(registry& reg, std::unordered_map<int, std::size_t>& client_entity_ids,
                       int client_id);

    bool check_all_players_dead(registry& reg,
                                const std::unordered_map<int, std::size_t>& client_entity_ids);

    void respawn_dead_players(registry& reg,
                              std::unordered_map<int, std::size_t>& client_entity_ids);
};

}  // namespace server
