#pragma once

#include "../../engine/ecs/components.hpp"
#include "../../engine/ecs/registry.hpp"
#include "../../game-lib/include/components/game_components.hpp"
#include "../../game-lib/include/components/logic_components.hpp"

#include <iostream>
#include <unordered_map>

namespace server {

class WeaponHandler {
public:
    WeaponHandler() = default;
    ~WeaponHandler() = default;

    bool handle_weapon_upgrade_choice(registry& reg,
                                      const std::unordered_map<int, std::size_t>& client_entity_ids,
                                      int client_id, uint8_t upgrade_choice);

private:
    std::optional<entity>
    get_player_entity(registry& reg, const std::unordered_map<int, std::size_t>& client_entity_ids,
                      int client_id);
};

}  // namespace server
