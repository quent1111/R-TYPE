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
                        
                        float angle = std::atan2(dy, dx);
                        
                        if (angle < 0.0f) {
                            angle += 6.28318f;
                        }
                        
                        const float min_angle = 2.356f;
                        const float max_angle = 3.927f;
                        
                        if (angle < min_angle || angle > max_angle) {
                            angle = 3.14159f;
                        }
                        
                        float vx = std::cos(angle) * wpn.projectile_speed;
                        float vy = std::sin(angle) * wpn.projectile_speed;
                        createCustomProjectile(reg, pos.x - 20.0f, pos.y, vx, vy, wpn.damage, attack);
                    }
                } else if (attack.pattern_type == "spread") {
                    float base_angle = -180.0f;
                    int count = attack.projectile_count;
                    float spread = attack.spread_angle;
                    
                    for (int p = 0; p < count; ++p) {
                        float angle_offset = 0.0f;
                        if (count > 1) {
                            angle_offset = (static_cast<float>(p) / static_cast<float>(count - 1) - 0.5f) * spread;
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
                    
                    float angle = std::atan2(dy, dx);
                    
                    if (angle < 0.0f) {
                        angle += 6.28318f;
                    }
                    
                    const float min_angle = 2.356f;
                    const float max_angle = 3.927f;

                    if (angle < min_angle || angle > max_angle) {
                        angle = 3.14159f;
                    }
                    
                    float vx = std::cos(angle) * wpn.projectile_speed * 1.5f;
                    float vy = std::sin(angle) * wpn.projectile_speed * 1.5f;
                    createEnemy2Projectile(reg, pos.x - 20.0f, pos.y, vx, vy, wpn.damage);
                }
            } else if (entity_tag.type == RType::EntityType::Enemy3) {
                float nearest_dist = -1.0f;
                float target_x = -1.0f;
                float target_y = -1.0f;
                bool found_valid_target = false;

                for (std::size_t j = 0; j < player_tags.size(); ++j) {
                    if (player_tags[j].has_value() && j < positions.size() && positions[j].has_value()) {
                        const auto& player_pos = positions[j].value();
                        float dx = player_pos.x - pos.x;
                        float dy = player_pos.y - pos.y;
                        if (dx > 0.0f) {
                            continue;
                        }
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
                    
                    float base_angle = std::atan2(dy, dx);
                    
                    if (base_angle < 0.0f) {
                        base_angle += 6.28318f;
                    }

                    const float min_angle = 2.356f;
                    const float max_angle = 3.927f;

                    if (base_angle < min_angle || base_angle > max_angle) {
                        base_angle = 3.14159f;
                    }
                    
                    float base_vx = std::cos(base_angle) * 400.0f;
                    float base_vy = std::sin(base_angle) * 400.0f;

                    static int shot_counter = 0;
                    for (int burst = 0; burst < 3; burst++) {
                        int projectile_type = (shot_counter + burst) % 3;
                        float offset_x = static_cast<float>(burst - 1) * 50.0f;
                        float angle_offset = static_cast<float>(burst - 1) * 0.087f;
                        float vx = base_vx * std::cos(angle_offset) - base_vy * std::sin(angle_offset);
                        float vy = base_vx * std::sin(angle_offset) + base_vy * std::cos(angle_offset);
                        createEnemy3Projectile(reg, pos.x - 20.0f + offset_x, pos.y - 15.0f, vx, vy, wpn.damage, projectile_type);
                    }
                    shot_counter++;
                }
            } else if (entity_tag.type == RType::EntityType::FlyingEnemy) {
                for (int burst = 0; burst < 3; burst++) {
                    float offset_y = static_cast<float>(burst - 1) * 20.0f;
                    createFlyingEnemyProjectile(reg, pos.x - 20.0f, pos.y + offset_y, -wpn.projectile_speed, 0.0f, wpn.damage);
                }
            } else if (entity_tag.type == RType::EntityType::Enemy4) {
                float angle_up = 3.14159f - 0.5f;
                float angle_down = 3.14159f + 0.5f;
                float vx_up = std::cos(angle_up) * wpn.projectile_speed;
                float vy_up = std::sin(angle_up) * wpn.projectile_speed;
                createEnemy4Projectile(reg, pos.x - 10.0f, pos.y - 10.0f, vx_up, vy_up, wpn.damage);
                float vx_down = std::cos(angle_down) * wpn.projectile_speed;
                float vy_down = std::sin(angle_down) * wpn.projectile_speed;
                createEnemy4Projectile(reg, pos.x - 10.0f, pos.y + 30.0f, vx_down, vy_down, wpn.damage);
            } else if (entity_tag.type == RType::EntityType::Enemy5) {
                createEnemy5Projectile(reg, pos.x - 20.0f, pos.y + 30.0f, -wpn.projectile_speed, 0.0f, wpn.damage);
            } else {
                createEnemyProjectile(reg, pos.x - 20.0f, pos.y, -wpn.projectile_speed, 0.0f, wpn.damage);
            }
            wpn.reset_shot_timer();
        }
    }
}
