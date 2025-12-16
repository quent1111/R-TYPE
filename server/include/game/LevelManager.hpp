#pragma once

#include "../../engine/ecs/registry.hpp"
#include "../../game-lib/include/components/game_components.hpp"
#include "../../game-lib/include/components/logic_components.hpp"

#include <iostream>
#include <optional>

namespace server {

class LevelManager {
public:
    LevelManager() = default;
    ~LevelManager() = default;

    bool check_level_completion(registry& reg);

    void advance_level(registry& reg);

    void clear_enemies_and_projectiles(registry& reg, std::optional<entity>& boss_entity);

    uint8_t get_current_level(registry& reg);

    std::optional<size_t> get_level_manager_index(registry& reg);
};

}  // namespace server
