#pragma once

#include "../../engine/ecs/components.hpp"
#include "../../engine/ecs/registry.hpp"
#include "../../game-lib/include/components/game_components.hpp"
#include "../../game-lib/include/components/logic_components.hpp"
#include "network/UDPServer.hpp"

#include <iostream>
#include <unordered_map>
#include <unordered_set>

namespace server {

class PowerupHandler {
public:
    PowerupHandler() = default;
    ~PowerupHandler() = default;

    void handle_powerup_choice(registry& reg,
                               const std::unordered_map<int, std::size_t>& client_entity_ids,
                               std::unordered_set<int>& players_who_chose_powerup, int client_id,
                               uint8_t powerup_choice);

    void handle_powerup_activate(registry& reg,
                                 const std::unordered_map<int, std::size_t>& client_entity_ids,
                                 int client_id, uint8_t powerup_type);

    int count_alive_players(registry& reg,
                            const std::unordered_map<int, std::size_t>& client_entity_ids);

private:
    std::optional<entity>
    get_player_entity(registry& reg, const std::unordered_map<int, std::size_t>& client_entity_ids,
                      int client_id);
};

}  // namespace server
