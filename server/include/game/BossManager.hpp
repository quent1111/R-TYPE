#pragma once

#include "../../engine/ecs/entity.hpp"
#include "../../engine/ecs/registry.hpp"
#include "../../game-lib/include/components/game_components.hpp"
#include "../../game-lib/include/components/logic_components.hpp"
#include "../../src/Common/Opcodes.hpp"

#include <cmath>

#include <iostream>
#include <optional>
#include <unordered_map>

namespace server {

class BossManager {
public:
    BossManager() = default;
    ~BossManager() = default;

    void spawn_boss_level_5(registry& reg, std::optional<entity>& boss_entity,
                            float& boss_animation_timer, float& boss_shoot_timer,
                            bool& boss_animation_complete, bool& boss_entrance_complete,
                            float& boss_target_x);

    void update_boss_behavior(registry& reg, std::optional<entity>& boss_entity,
                              const std::unordered_map<int, std::size_t>& client_entity_ids,
                              float& boss_animation_timer, float& boss_shoot_timer,
                              float boss_shoot_cooldown, bool& boss_animation_complete,
                              bool& boss_entrance_complete, float boss_target_x,
                              int& boss_shoot_counter, float dt);

    void update_homing_enemies(registry& reg,
                               const std::unordered_map<int, std::size_t>& client_entity_ids,
                               float dt);

private:
    void boss_shoot_projectile(registry& reg, std::optional<entity>& boss_entity,
                               const std::unordered_map<int, std::size_t>& client_entity_ids);

    void boss_spawn_homing_enemy(registry& reg, std::optional<entity>& boss_entity);
};

}  // namespace server
