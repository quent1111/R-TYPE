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
#include <random>

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

    void spawn_boss_level_10(registry& reg, std::optional<entity>& serpent_controller_entity);

    void update_serpent_boss(registry& reg, 
                             std::optional<entity>& serpent_controller_entity,
                             const std::unordered_map<int, std::size_t>& client_entity_ids,
                             float dt);

    std::pair<int, int> get_boss_health(registry& reg, std::optional<entity>& boss_entity, 
                                        std::optional<entity>& serpent_controller);

private:
    void boss_shoot_projectile(registry& reg, std::optional<entity>& boss_entity,
                               const std::unordered_map<int, std::size_t>& client_entity_ids);

    void boss_spawn_homing_enemy(registry& reg, std::optional<entity>& boss_entity);

    void spawn_serpent_nest(registry& reg, serpent_boss_controller& controller);
    void spawn_serpent_part(registry& reg, serpent_boss_controller& controller, 
                            SerpentPartType type, int index, float x, float y,
                            std::optional<entity> parent);
    void spawn_serpent_scale_on_body(registry& reg, serpent_boss_controller& controller,
                                     entity body_entity, float x, float y, int scale_index);
    void update_serpent_spawn_animation(registry& reg, serpent_boss_controller& controller, float dt);
    void update_serpent_movement(registry& reg, serpent_boss_controller& controller, float dt);
    void update_serpent_parts_follow(registry& reg, serpent_boss_controller& controller, float dt);
    void update_serpent_rotations(registry& reg, serpent_boss_controller& controller,
                                  const std::unordered_map<int, std::size_t>& client_entity_ids);
    void handle_serpent_part_damage(registry& reg, serpent_boss_controller& controller, 
                                    entity part_entity, int damage);
    
    void update_serpent_attacks(registry& reg, serpent_boss_controller& controller,
                                const std::unordered_map<int, std::size_t>& client_entity_ids,
                                float dt);
    void serpent_scale_shoot(registry& reg, serpent_boss_controller& controller,
                             const std::unordered_map<int, std::size_t>& client_entity_ids);
    void serpent_scream_attack(registry& reg, serpent_boss_controller& controller);
    void serpent_spawn_homing(registry& reg, float x, float y, float target_x, float target_y);
    void serpent_laser_attack(registry& reg, serpent_boss_controller& controller,
                              const std::unordered_map<int, std::size_t>& client_entity_ids,
                              float dt);
    
    std::mt19937 rng_{std::random_device{}()};
};

}  // namespace server
