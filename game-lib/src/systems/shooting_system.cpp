#include "systems/shooting_system.hpp"
#include "ecs/components.hpp"
#include "components/game_components.hpp"
#include "entities/projectile_factory.hpp"
#include <cmath>

void shootingSystem(registry& reg, float dt) {
    auto& weapons = reg.get_components<weapon>();

    for (std::size_t i = 0; i < weapons.size(); ++i) {
        auto& weapon_opt = weapons[i];

        if (weapon_opt) {
            auto& wpn = weapon_opt.value();
            wpn.update(dt);
        }
    }
}

void enemyShootingSystem(registry& reg, float /*dt*/) {
    auto& enemies = reg.get_components<enemy_tag>();
    auto& weapons = reg.get_components<weapon>();
    auto& positions = reg.get_components<position>();
    auto& entity_tags = reg.get_components<entity_tag>();
    auto& player_tags = reg.get_components<player_tag>();

    for (std::size_t i = 0; i < enemies.size(); ++i) {
        if (!enemies[i].has_value()) continue;

        if (i >= weapons.size() || !weapons[i].has_value()) continue;
        if (i >= positions.size() || !positions[i].has_value()) continue;
        if (i >= entity_tags.size() || !entity_tags[i].has_value()) continue;

        auto& wpn = weapons[i].value();
        auto& pos = positions[i].value();
        auto& entity_tag = entity_tags[i].value();

        if (wpn.can_shoot()) {
            // Check if this is Enemy2 (type 0x06)
            if (entity_tag.type == RType::EntityType::Enemy2) {
                // Find nearest player
                float nearest_dist = -1.0f;
                float target_x = pos.x - 300.0f;  // Default target if no player
                float target_y = pos.y;

                for (std::size_t j = 0; j < player_tags.size(); ++j) {
                    if (player_tags[j].has_value() && j < positions.size() && positions[j].has_value()) {
                        const auto& player_pos = positions[j].value();
                        float dx = player_pos.x - pos.x;
                        float dy = player_pos.y - pos.y;
                        float dist = std::sqrt(dx * dx + dy * dy);

                        if (nearest_dist < 0.0f || dist < nearest_dist) {
                            nearest_dist = dist;
                            target_x = player_pos.x;
                            target_y = player_pos.y;
                        }
                    }
                }

                // Calculate direction to player
                float dx = target_x - pos.x;
                float dy = target_y - pos.y;
                float magnitude = std::sqrt(dx * dx + dy * dy);

                if (magnitude > 0.0f) {
                    float vx = (dx / magnitude) * wpn.projectile_speed * 1.5f;  // 50% faster
                    float vy = (dy / magnitude) * wpn.projectile_speed * 1.5f;
                    createEnemy2Projectile(reg, pos.x - 20.0f, pos.y, vx, vy, wpn.damage);
                }
            } else {
                // Regular enemy (type 0x02) - shoot straight left
                createEnemyProjectile(reg, pos.x - 20.0f, pos.y, -wpn.projectile_speed, 0.0f, wpn.damage);
            }
            wpn.reset_shot_timer();
        }
    }
}
