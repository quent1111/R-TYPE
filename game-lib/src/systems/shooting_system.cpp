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
    auto& custom_attacks = reg.get_components<custom_attack_config>();

    for (std::size_t i = 0; i < enemies.size(); ++i) {
        if (!enemies[i].has_value()) continue;

        if (i >= weapons.size() || !weapons[i].has_value()) continue;
        if (i >= positions.size() || !positions[i].has_value()) continue;
        if (i >= entity_tags.size() || !entity_tags[i].has_value()) continue;

        auto& wpn = weapons[i].value();
        auto& pos = positions[i].value();
        auto& entity_tag = entity_tags[i].value();

        if (wpn.can_shoot()) {
            bool has_custom_attack = (i < custom_attacks.size() && custom_attacks[i].has_value());
            
            if (has_custom_attack) {
                const auto& attack = custom_attacks[i].value();
                
                if (attack.pattern_type == "targeted") {
                    float nearest_dist = -1.0f;
                    float target_x = -1.0f;
                    float target_y = -1.0f;
                    bool found_valid_target = false;

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
                                found_valid_target = true;
                            }
                        }
                    }

                    if (found_valid_target) {
                        float dx = target_x - pos.x;
                        float dy = target_y - pos.y;
                        float magnitude = std::sqrt(dx * dx + dy * dy);

                        if (magnitude > 0.0f) {
                            float vx = (dx / magnitude) * wpn.projectile_speed;
                            float vy = (dy / magnitude) * wpn.projectile_speed;
                            createCustomProjectile(reg, pos.x - 20.0f, pos.y, vx, vy, wpn.damage, attack);
                        }
                    }
                } else if (attack.pattern_type == "spread") {
                    float base_angle = -180.0f;
                    int count = attack.projectile_count;
                    float spread = attack.spread_angle;
                    
                    for (int p = 0; p < count; ++p) {
                        float angle_offset = 0.0f;
                        if (count > 1) {
                            angle_offset = (static_cast<float>(p) / (count - 1) - 0.5f) * spread;
                        }
                        float angle_rad = (base_angle + angle_offset) * 3.14159f / 180.0f;
                        float vx = std::cos(angle_rad) * wpn.projectile_speed;
                        float vy = std::sin(angle_rad) * wpn.projectile_speed;
                        createCustomProjectile(reg, pos.x - 20.0f, pos.y, vx, vy, wpn.damage, attack);
                    }
                } else {
                    createCustomProjectile(reg, pos.x - 20.0f, pos.y, -wpn.projectile_speed, 0.0f, wpn.damage, attack);
                }
            } else if (entity_tag.type == RType::EntityType::Enemy2) {
                float nearest_dist = -1.0f;
                float target_x = -1.0f;
                float target_y = -1.0f;
                bool found_valid_target = false;

                for (std::size_t j = 0; j < player_tags.size(); ++j) {
                    if (player_tags[j].has_value() && j < positions.size() && positions[j].has_value()) {
                        const auto& player_pos = positions[j].value();
                        float dx = player_pos.x - pos.x;
                        float dy = player_pos.y - pos.y;

                        if (dx < 0.0f || std::abs(dx) < 100.0f) {
                            float dist = std::sqrt(dx * dx + dy * dy);

                            if (nearest_dist < 0.0f || dist < nearest_dist) {
                                nearest_dist = dist;
                                target_x = player_pos.x;
                                target_y = player_pos.y;
                                found_valid_target = true;
                            }
                        }
                    }
                }

                if (found_valid_target) {
                    float dx = target_x - pos.x;
                    float dy = target_y - pos.y;
                    float magnitude = std::sqrt(dx * dx + dy * dy);

                    if (magnitude > 0.0f) {
                        float vx = (dx / magnitude) * wpn.projectile_speed * 1.5f;
                        float vy = (dy / magnitude) * wpn.projectile_speed * 1.5f;
                        createEnemy2Projectile(reg, pos.x - 20.0f, pos.y, vx, vy, wpn.damage);
                    }
                }
            } else {
                createEnemyProjectile(reg, pos.x - 20.0f, pos.y, -wpn.projectile_speed, 0.0f, wpn.damage);
            }
            wpn.reset_shot_timer();
        }
    }
}
