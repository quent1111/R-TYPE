#include "systems/collision_system.hpp"
#include "components/logic_components.hpp"
#include "components/game_components.hpp"
#include "entities/explosion_factory.hpp"
#include <iostream>

void collisionSystem(registry& reg) {
    auto& positions = reg.get_components<position>();
    auto& collision_boxes = reg.get_components<collision_box>();
    auto& multi_hitboxes = reg.get_components<multi_hitbox>();
    auto& damage_contacts = reg.get_components<damage_on_contact>();
    auto& healths = reg.get_components<health>();
    auto& enemy_tags = reg.get_components<enemy_tag>();
    auto& boss_tags = reg.get_components<boss_tag>();
    auto& projectile_tags = reg.get_components<projectile_tag>();
    auto& player_tags = reg.get_components<player_tag>();
    auto& level_managers = reg.get_components<level_manager>();
    auto& shields = reg.get_components<shield>();
    auto& damage_flashes = reg.get_components<damage_flash_component>();

    for (std::size_t p = 0; p < positions.size() && p < player_tags.size(); ++p) {
        if (player_tags[p] && positions[p] && shields[p]) {
            auto& player_pos = positions[p].value();
            auto& player_shield = shields[p].value();

            if (player_shield.is_active()) {
                for (std::size_t e = 0; e < positions.size() && e < enemy_tags.size(); ++e) {
                    if (enemy_tags[e] && positions[e] && healths[e]) {
                        auto& enemy_pos = positions[e].value();
                        auto& enemy_hp = healths[e].value();

                        if (player_shield.is_enemy_in_range(enemy_pos.x, enemy_pos.y, player_pos.x, player_pos.y)) {
                            enemy_hp.current = 0;

                            createExplosion(reg, enemy_pos.x, enemy_pos.y);

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

                for (std::size_t b = 0; b < positions.size() && b < boss_tags.size(); ++b) {
                    if (boss_tags[b] && positions[b] && healths[b]) {
                        auto& boss_pos = positions[b].value();
                        auto& boss_hp = healths[b].value();

                        if (player_shield.is_enemy_in_range(boss_pos.x, boss_pos.y, player_pos.x, player_pos.y)) {
                            boss_hp.current -= 10;
                            if (boss_hp.current < 0) boss_hp.current = 0;

                            if (b < damage_flashes.size() && damage_flashes[b].has_value()) {
                                damage_flashes[b]->trigger();
                            }

                            createExplosion(reg, boss_pos.x + (player_pos.x - boss_pos.x) * 0.3f, 
                                          boss_pos.y + (player_pos.y - boss_pos.y) * 0.3f);

                            if (boss_hp.is_dead()) {
                                createExplosion(reg, boss_pos.x, boss_pos.y);

                                for (size_t k = 0; k < level_managers.size(); ++k) {
                                    if (level_managers[k].has_value()) {
                                        auto& lvl = level_managers[k].value();
                                        int remaining = lvl.enemies_needed_for_next_level - lvl.enemies_killed_this_level;
                                        for (int m = 0; m < remaining; ++m) {
                                            lvl.on_enemy_killed();
                                        }
                                        break;
                                    }
                                }

                                auto boss_entity = reg.entity_from_index(b);
                                reg.remove_component<entity_tag>(boss_entity);
                                reg.kill_entity(boss_entity);
                            }
                        }
                    }
                }
            }
        }
    }

    for (std::size_t i = 0; i < positions.size() && i < projectile_tags.size(); ++i) {
        if (projectile_tags[i] && positions[i] && collision_boxes[i] && damage_contacts[i]) {
            bool is_enemy_projectile = (i < enemy_tags.size() && enemy_tags[i].has_value());

            if (is_enemy_projectile) continue;

            auto& proj_pos = positions[i].value();
            auto& proj_box = collision_boxes[i].value();
            auto& proj_dmg = damage_contacts[i].value();

            for (std::size_t j = 0; j < positions.size() && j < enemy_tags.size(); ++j) {
                if (i == j) continue;

                if (j < projectile_tags.size() && projectile_tags[j].has_value()) continue;

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
                        enemy_hp.current -= proj_dmg.damage_amount;
                        if (enemy_hp.current < 0) enemy_hp.current = 0;

                        if (proj_dmg.destroy_on_hit) {
                            auto proj_entity = reg.entity_from_index(i);
                            reg.remove_component<entity_tag>(proj_entity);
                            reg.kill_entity(proj_entity);
                        }

                        if (enemy_hp.is_dead()) {
                            createExplosion(reg, enemy_pos.x, enemy_pos.y);

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

            for (std::size_t j = 0; j < positions.size() && j < boss_tags.size(); ++j) {
                if (i == j) continue;

                if (j < projectile_tags.size() && projectile_tags[j].has_value()) continue;

                if (boss_tags[j] && positions[j] && healths[j]) {
                    auto& boss_pos = positions[j].value();
                    auto& boss_hp = healths[j].value();

                    float p_left = proj_pos.x + proj_box.offset_x;
                    float p_top = proj_pos.y + proj_box.offset_y;
                    float p_right = p_left + proj_box.width;
                    float p_bottom = p_top + proj_box.height;

                    bool hit_detected = false;

                    if (j < multi_hitboxes.size() && multi_hitboxes[j].has_value()) {
                        auto& boss_multi = multi_hitboxes[j].value();
                        for (const auto& part : boss_multi.parts) {
                            float b_left = boss_pos.x + part.offset_x;
                            float b_top = boss_pos.y + part.offset_y;
                            float b_right = b_left + part.width;
                            float b_bottom = b_top + part.height;

                            if (p_left < b_right && p_right > b_left &&
                                p_top < b_bottom && p_bottom > b_top) {
                                hit_detected = true;
                                break;
                            }
                        }
                    } else if (j < collision_boxes.size() && collision_boxes[j].has_value()) {
                        auto& boss_box = collision_boxes[j].value();
                        float b_left = boss_pos.x + boss_box.offset_x;
                        float b_top = boss_pos.y + boss_box.offset_y;
                        float b_right = b_left + boss_box.width;
                        float b_bottom = b_top + boss_box.height;

                        if (p_left < b_right && p_right > b_left &&
                            p_top < b_bottom && p_bottom > b_top) {
                            hit_detected = true;
                        }
                    }

                    if (hit_detected) {
                        boss_hp.current -= proj_dmg.damage_amount;
                        if (boss_hp.current < 0) boss_hp.current = 0;

                        if (j < damage_flashes.size() && damage_flashes[j].has_value()) {
                            damage_flashes[j]->trigger();
                        }

                        createExplosion(reg, proj_pos.x, proj_pos.y);

                        if (proj_dmg.destroy_on_hit) {
                            auto proj_entity = reg.entity_from_index(i);
                            reg.remove_component<entity_tag>(proj_entity);
                            reg.kill_entity(proj_entity);
                        }

                        if (boss_hp.is_dead()) {
                            createExplosion(reg, boss_pos.x, boss_pos.y);

                            for (size_t k = 0; k < level_managers.size(); ++k) {
                                if (level_managers[k].has_value()) {
                                    auto& lvl = level_managers[k].value();
                                    int remaining = lvl.enemies_needed_for_next_level - lvl.enemies_killed_this_level;
                                    for (int m = 0; m < remaining; ++m) {
                                        lvl.on_enemy_killed();
                                    }
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

    for (std::size_t i = 0; i < positions.size() && i < projectile_tags.size(); ++i) {
        if (projectile_tags[i] && positions[i] && collision_boxes[i]) {
            bool is_enemy_projectile_i = (i < enemy_tags.size() && enemy_tags[i].has_value());

            auto& proj_pos_i = positions[i].value();
            auto& proj_box_i = collision_boxes[i].value();

            for (std::size_t j = i + 1; j < positions.size() && j < projectile_tags.size(); ++j) {
                if (projectile_tags[j] && positions[j] && collision_boxes[j]) {
                    bool is_enemy_projectile_j = (j < enemy_tags.size() && enemy_tags[j].has_value());

                    if (is_enemy_projectile_i != is_enemy_projectile_j) {
                        auto& proj_pos_j = positions[j].value();
                        auto& proj_box_j = collision_boxes[j].value();

                        float pi_left = proj_pos_i.x + proj_box_i.offset_x;
                        float pi_top = proj_pos_i.y + proj_box_i.offset_y;
                        float pi_right = pi_left + proj_box_i.width;
                        float pi_bottom = pi_top + proj_box_i.height;

                        float pj_left = proj_pos_j.x + proj_box_j.offset_x;
                        float pj_top = proj_pos_j.y + proj_box_j.offset_y;
                        float pj_right = pj_left + proj_box_j.width;
                        float pj_bottom = pj_top + proj_box_j.height;

                        if (pi_left < pj_right && pi_right > pj_left &&
                            pi_top < pj_bottom && pi_bottom > pj_top) {

                            std::size_t enemy_idx = is_enemy_projectile_i ? i : j;
                            std::size_t player_proj_idx = is_enemy_projectile_i ? j : i;

                            auto proj_entity_player = reg.entity_from_index(player_proj_idx);
                            reg.remove_component<entity_tag>(proj_entity_player);
                            reg.kill_entity(proj_entity_player);

                            if (enemy_idx < healths.size() && healths[enemy_idx].has_value()) {
                                auto& enemy_hp = healths[enemy_idx].value();
                                enemy_hp.current -= 1;
                                if (enemy_hp.current <= 0) {
                                    auto proj_entity_enemy = reg.entity_from_index(enemy_idx);
                                    reg.remove_component<entity_tag>(proj_entity_enemy);
                                    reg.kill_entity(proj_entity_enemy);
                                }
                            } else {
                                auto proj_entity_enemy = reg.entity_from_index(enemy_idx);
                                reg.remove_component<entity_tag>(proj_entity_enemy);
                                reg.kill_entity(proj_entity_enemy);
                            }

                            break;
                        }
                    }
                }
            }
        }
    }

    for (std::size_t i = 0; i < positions.size() && i < projectile_tags.size(); ++i) {
        if (projectile_tags[i] && positions[i] && collision_boxes[i] && damage_contacts[i]) {
            bool is_enemy_projectile = (i < enemy_tags.size() && enemy_tags[i].has_value());

            if (!is_enemy_projectile) continue;

            auto& proj_pos = positions[i].value();
            auto& proj_box = collision_boxes[i].value();
            auto& proj_dmg = damage_contacts[i].value();

            for (std::size_t j = 0; j < positions.size() && j < player_tags.size(); ++j) {
                if (i == j) continue;

                if (player_tags[j] && positions[j] && collision_boxes[j] && healths[j]) {
                    auto& player_pos = positions[j].value();
                    auto& player_box = collision_boxes[j].value();
                    auto& player_hp = healths[j].value();

                    float p_left = proj_pos.x + proj_box.offset_x;
                    float p_top = proj_pos.y + proj_box.offset_y;
                    float p_right = p_left + proj_box.width;
                    float p_bottom = p_top + proj_box.height;

                    float pl_left = player_pos.x + player_box.offset_x;
                    float pl_top = player_pos.y + player_box.offset_y;
                    float pl_right = pl_left + player_box.width;
                    float pl_bottom = pl_top + player_box.height;

                    if (p_left < pl_right && p_right > pl_left &&
                        p_top < pl_bottom && p_bottom > pl_top) {

                        bool has_active_shield = false;
                        if (j < shields.size() && shields[j].has_value()) {
                            has_active_shield = shields[j]->is_active();
                        }

                        if (has_active_shield) {
                            std::cout << "[Collision] Projectile blocked by shield!" << std::endl;
                        } else {
                            player_hp.current -= proj_dmg.damage_amount;
                            if (player_hp.current < 0) player_hp.current = 0;

                            if (player_hp.is_dead()) {
                                std::cout << "[Collision] Player killed!" << std::endl;
                                createExplosion(reg, player_pos.x, player_pos.y);
                            }
                        }

                        if (proj_dmg.destroy_on_hit) {
                            auto proj_entity = reg.entity_from_index(i);
                            reg.remove_component<entity_tag>(proj_entity);
                            reg.kill_entity(proj_entity);
                        }

                        break;
                    }
                }
            }
        }
    }
}