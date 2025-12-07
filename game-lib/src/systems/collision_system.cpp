#include "systems/collision_system.hpp"
#include "components/logic_components.hpp"
#include "components/game_components.hpp"
#include "entities/explosion_factory.hpp"
#include <iostream>

void collisionSystem(registry& reg) {
    auto& positions = reg.get_components<position>();
    auto& collision_boxes = reg.get_components<collision_box>();
    auto& damage_contacts = reg.get_components<damage_on_contact>();
    auto& healths = reg.get_components<health>();
    auto& enemy_tags = reg.get_components<enemy_tag>();
    auto& projectile_tags = reg.get_components<projectile_tag>();
    auto& player_tags = reg.get_components<player_tag>();
    auto& level_managers = reg.get_components<level_manager>();
    auto& shields = reg.get_components<shield>();

    // Shield collision - tuer les ennemis qui entrent dans le shield
    for (std::size_t p = 0; p < positions.size() && p < player_tags.size(); ++p) {
        if (player_tags[p] && positions[p] && shields[p]) {
            auto& player_pos = positions[p].value();
            auto& player_shield = shields[p].value();
            
            if (player_shield.is_active()) {
                // Vérifier tous les ennemis
                for (std::size_t e = 0; e < positions.size() && e < enemy_tags.size(); ++e) {
                    if (enemy_tags[e] && positions[e] && healths[e]) {
                        auto& enemy_pos = positions[e].value();
                        auto& enemy_hp = healths[e].value();
                        
                        if (player_shield.is_enemy_in_range(enemy_pos.x, enemy_pos.y, player_pos.x, player_pos.y)) {
                            // Ennemi dans le shield - le tuer instantanément
                            enemy_hp.current = 0;
                            
                            createExplosion(reg, enemy_pos.x, enemy_pos.y);
                            
                            // Incrémenter le compteur d'ennemis tués
                            for (size_t k = 0; k < level_managers.size(); ++k) {
                                if (level_managers[k].has_value()) {
                                    level_managers[k]->on_enemy_killed();
                                    break;
                                }
                            }
                            
                            auto enemy_entity = reg.entity_from_index(e);
                            reg.remove_component<entity_tag>(enemy_entity);
                            reg.kill_entity(enemy_entity);
                        }
                    }
                }
            }
        }
    }

    // Collision projectile-ennemi (existant)
    for (std::size_t i = 0; i < positions.size() && i < projectile_tags.size(); ++i) {
        if (projectile_tags[i] && positions[i] && collision_boxes[i] && damage_contacts[i]) {
            auto& proj_pos = positions[i].value();
            auto& proj_box = collision_boxes[i].value();
            auto& proj_dmg = damage_contacts[i].value();

            for (std::size_t j = 0; j < positions.size() && j < enemy_tags.size(); ++j) {
                if (i == j) continue;

                if (enemy_tags[j] && positions[j] && collision_boxes[j] && healths[j]) {
                    auto& enemy_pos = positions[j].value();
                    auto& enemy_box = collision_boxes[j].value();
                    auto& enemy_hp = healths[j].value();

                    float p_left = proj_pos.x + proj_box.offset_x;
                    float p_top = proj_pos.y + proj_box.offset_y;
                    float p_right = p_left + proj_box.width;
                    float p_bottom = p_top + proj_box.height;

                    float e_left = enemy_pos.x + enemy_box.offset_x;
                    float e_top = enemy_pos.y + enemy_box.offset_y;
                    float e_right = e_left + enemy_box.width;
                    float e_bottom = e_top + enemy_box.height;

                    if (p_left < e_right && p_right > e_left &&
                        p_top < e_bottom && p_bottom > e_top) {
                        // Collision détectée
                        enemy_hp.current -= proj_dmg.damage_amount;
                        if (enemy_hp.current < 0) enemy_hp.current = 0;

                        if (proj_dmg.destroy_on_hit) {
                            auto proj_entity = reg.entity_from_index(i);
                            reg.remove_component<entity_tag>(proj_entity);
                            reg.kill_entity(proj_entity);
                        }

                        if (enemy_hp.is_dead()) {
                            createExplosion(reg, enemy_pos.x, enemy_pos.y);
                            
                            // Incrémenter le compteur d'ennemis tués dans le level manager
                            for (size_t k = 0; k < level_managers.size(); ++k) {
                                if (level_managers[k].has_value()) {
                                    level_managers[k]->on_enemy_killed();
                                    break;
                                }
                            }
                        }
                        break;
                    }
                }
            }
        }
    }
}