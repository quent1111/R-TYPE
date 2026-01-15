#include "game/BossManager.hpp"

#include <array>

namespace server {

void BossManager::spawn_boss_level_5(registry& reg, std::optional<entity>& boss_entity,
                                     float& boss_animation_timer, float& boss_shoot_timer,
                                     bool& boss_animation_complete, bool& boss_entrance_complete,
                                     float& boss_target_x) {
    float boss_spawn_x = 2400.0f;
    float boss_y = 540.0f;
    boss_target_x = 1500.0f;

    entity boss = reg.spawn_entity();

    reg.add_component(boss, position{boss_spawn_x, boss_y});

    reg.add_component(boss, velocity{-150.0f, 0.0f});

    reg.add_component(boss, health{2000, 2000});

    reg.add_component(boss, damage_on_contact{50, false});

    multi_hitbox boss_hitboxes;
    boss_hitboxes.parts = {multi_hitbox::hitbox_part{250.0f, 250.0f, -200.0f, -350.0f},
                           multi_hitbox::hitbox_part{350.0f, 500.0f, 25.0f, -100.0f},
                           multi_hitbox::hitbox_part{200.0f, 200.0f, -100.0f, 220.0f}};
    reg.add_component(boss, boss_hitboxes);

    reg.add_component(boss, boss_tag{});
    reg.add_component(boss, entity_tag{RType::EntityType::Boss});
    reg.add_component(boss, damage_flash_component{0.15f});

    boss_entity = boss;
    boss_animation_timer = 0.0f;
    boss_shoot_timer = 0.0f;
    boss_animation_complete = false;
    boss_entrance_complete = false;
}

void BossManager::update_boss_behavior(
    registry& reg, std::optional<entity>& boss_entity,
    const std::unordered_map<int, std::size_t>& client_entity_ids, float& boss_animation_timer,
    float& boss_shoot_timer, float boss_shoot_cooldown, bool& boss_animation_complete,
    bool& boss_entrance_complete, float boss_target_x, int& boss_shoot_counter, float dt) {
    if (!boss_entity.has_value()) {
        return;
    }

    auto boss_health_opt = reg.get_component<health>(boss_entity.value());
    if (!boss_health_opt.has_value() || boss_health_opt->current <= 0) {
        try {
            auto boss_ent = boss_entity.value();
            reg.remove_component<entity_tag>(boss_ent);
            reg.kill_entity(boss_ent);
        } catch (...) {}
        boss_entity = std::nullopt;
        return;
    }

    if (!boss_entrance_complete) {
        auto& positions = reg.get_components<position>();
        auto& velocities = reg.get_components<velocity>();

        std::size_t boss_idx = static_cast<std::size_t>(boss_entity.value());

        if (boss_idx < positions.size() && positions[boss_idx].has_value()) {
            if (positions[boss_idx]->x <= boss_target_x) {
                positions[boss_idx]->x = boss_target_x;

                if (boss_idx < velocities.size() && velocities[boss_idx].has_value()) {
                    velocities[boss_idx]->vx = 0.0f;
                    velocities[boss_idx]->vy = 0.0f;
                }

                boss_entrance_complete = true;
            }
        }
        return;
    }

    boss_animation_timer += dt;

    if (boss_animation_timer >= 2.5f && !boss_animation_complete) {
        boss_animation_complete = true;
        boss_shoot_timer = 0.0f;
    }

    if (boss_animation_complete) {
        boss_shoot_timer += dt;

        if (boss_shoot_timer >= boss_shoot_cooldown) {
            boss_shoot_projectile(reg, boss_entity, client_entity_ids);
            boss_shoot_counter++;

            if (boss_shoot_counter >= 3) {
                boss_spawn_homing_enemy(reg, boss_entity);
                boss_shoot_counter = 0;
            }

            boss_shoot_timer = 0.0f;
        }
    }
}

void BossManager::boss_shoot_projectile(
    registry& reg, std::optional<entity>& boss_entity,
    const std::unordered_map<int, std::size_t>& client_entity_ids) {
    if (!boss_entity.has_value()) {
        return;
    }

    auto boss_pos_opt = reg.get_component<position>(boss_entity.value());
    if (!boss_pos_opt.has_value()) {
        return;
    }

    float boss_x = boss_pos_opt->x;
    float boss_y = boss_pos_opt->y;

    std::vector<std::pair<float, float>> player_positions;
    for (const auto& [client_id, entity_id] : client_entity_ids) {
        auto player = reg.entity_from_index(entity_id);
        auto player_pos_opt = reg.get_component<position>(player);
        auto player_health_opt = reg.get_component<health>(player);

        if (player_pos_opt.has_value() && player_health_opt.has_value() &&
            player_health_opt->current > 0) {
            player_positions.push_back({player_pos_opt->x, player_pos_opt->y});
        }
    }

    if (player_positions.empty()) {
        return;
    }

    for (const auto& [target_x, target_y] : player_positions) {
        float dx = target_x - boss_x;
        float dy = target_y - boss_y;
        float distance = std::sqrt(dx * dx + dy * dy);

        if (distance < 1.0f) {
            continue;
        }

        float speed = 400.0f;
        float vx = (dx / distance) * speed;
        float vy = (dy / distance) * speed;

        auto projectile = reg.spawn_entity();

        reg.add_component(projectile, position{boss_x - 10.0f, boss_y + 20.0f});

        reg.add_component(projectile, velocity{vx, vy});

        reg.add_component(projectile, health{3, 3});

        reg.add_component(projectile, collision_box{50.0f, 50.0f});
        reg.add_component(projectile, damage_on_contact{30, true});

        reg.add_component(projectile, projectile_tag{});
        reg.add_component(projectile, enemy_tag{});

        reg.add_component(projectile, entity_tag{static_cast<RType::EntityType>(0x07)});
    }
}

void BossManager::boss_spawn_homing_enemy(registry& reg, std::optional<entity>& boss_entity) {
    if (!boss_entity.has_value()) {
        return;
    }

    auto boss_pos_opt = reg.get_component<position>(boss_entity.value());
    if (!boss_pos_opt.has_value()) {
        return;
    }

    float boss_x = boss_pos_opt->x;
    float boss_y = boss_pos_opt->y;

    auto homing = reg.spawn_entity();

    reg.add_component(homing, position{boss_x - 50.0f, boss_y + 300.0f});

    reg.add_component(homing, velocity{-150.0f, 0.0f});

    reg.add_component(homing, health{10, 10});

    reg.add_component(homing, collision_box{35.0f, 35.0f});
    reg.add_component(homing, damage_on_contact{30, true});

    reg.add_component(homing, homing_component{250.0f, 3.0f});

    reg.add_component(homing, entity_tag{RType::EntityType::HomingEnemy});
}

void BossManager::update_homing_enemies(
    registry& reg, const std::unordered_map<int, std::size_t>& client_entity_ids, float dt) {
    auto& positions = reg.get_components<position>();
    auto& velocities = reg.get_components<velocity>();
    auto& homing_comps = reg.get_components<homing_component>();
    auto& entity_tags = reg.get_components<entity_tag>();
    for (std::size_t i = 0; i < entity_tags.size(); ++i) {
        if (!entity_tags[i].has_value() ||
            (entity_tags[i]->type != RType::EntityType::HomingEnemy &&
             entity_tags[i]->type != static_cast<RType::EntityType>(0x15)) ||
            !positions[i].has_value() || !velocities[i].has_value() ||
            !homing_comps[i].has_value()) {
            continue;
        }

        float homing_x = positions[i]->x;
        float homing_y = positions[i]->y;

        float closest_distance = std::numeric_limits<float>::max();
        float target_x = 0.0f;
        float target_y = 0.0f;
        bool found_target = false;

        for (const auto& [client_id, entity_id] : client_entity_ids) {
            auto player = reg.entity_from_index(entity_id);
            auto player_pos_opt = reg.get_component<position>(player);
            auto player_health_opt = reg.get_component<health>(player);

            if (player_pos_opt.has_value() && player_health_opt.has_value() &&
                player_health_opt->current > 0) {
                float dx = player_pos_opt->x - homing_x;
                float dy = player_pos_opt->y - homing_y;
                float distance = std::sqrt(dx * dx + dy * dy);

                if (distance < closest_distance) {
                    closest_distance = distance;
                    target_x = player_pos_opt->x;
                    target_y = player_pos_opt->y;
                    found_target = true;
                }
            }
        }

        if (found_target && closest_distance > 10.0f) {
            float dx = target_x - homing_x;
            float dy = target_y - homing_y;
            float distance = std::sqrt(dx * dx + dy * dy);

            float desired_vx = (dx / distance) * homing_comps[i]->speed;
            float desired_vy = (dy / distance) * homing_comps[i]->speed;

            float turn_factor = homing_comps[i]->turn_rate * dt;
            if (turn_factor > 1.0f)
                turn_factor = 1.0f;

            velocities[i]->vx += (desired_vx - velocities[i]->vx) * turn_factor;
            velocities[i]->vy += (desired_vy - velocities[i]->vy) * turn_factor;
        }
    }
}

void BossManager::spawn_boss_level_10(registry& reg,
                                      std::optional<entity>& serpent_controller_entity) {
    std::cout << "[BOSS] Level 10 - Spawning Serpent Boss!" << std::endl;

    entity controller = reg.spawn_entity();

    serpent_boss_controller ctrl(8000, 18, 6);
    ctrl.spawn_timer = 0.0f;
    ctrl.spawn_complete = false;
    ctrl.nest_visible = false;
    ctrl.movement_speed = 350.0f;
    ctrl.wave_frequency = 2.5f;
    ctrl.wave_amplitude = 50.0f;

    reg.add_component(controller, ctrl);
    reg.add_component(controller, position{960.0f, 1200.0f});
    reg.add_component(controller, boss_tag{});

    serpent_controller_entity = controller;
}

void BossManager::spawn_serpent_nest(registry& reg, serpent_boss_controller& controller) {
    entity nest = reg.spawn_entity();

    reg.add_component(nest, position{960.0f, 1000.0f});
    reg.add_component(nest, velocity{0.0f, 0.0f});
    reg.add_component(nest, entity_tag{RType::EntityType::SerpentNest});
    reg.add_component(nest, serpent_nest_tag{});

    controller.nest_entity = nest;
    controller.nest_visible = true;

    std::cout << "[BOSS] Serpent nest spawned at bottom of screen (half visible)" << std::endl;
}

void BossManager::spawn_serpent_part(registry& reg, serpent_boss_controller& controller,
                                     SerpentPartType type, int index, float x, float y,
                                     std::optional<entity> parent) {
    entity part = reg.spawn_entity();

    int part_health = 100;
    RType::EntityType entity_type = RType::EntityType::SerpentBody;

    switch (type) {
        case SerpentPartType::Head:
            part_health = 300;
            entity_type = RType::EntityType::SerpentHead;
            break;
        case SerpentPartType::Body:
            part_health = 100;
            entity_type = RType::EntityType::SerpentBody;
            break;
        case SerpentPartType::Scale:
            part_health = 150;
            entity_type = RType::EntityType::SerpentScale;
            break;
        case SerpentPartType::Tail:
            part_health = 80;
            entity_type = RType::EntityType::SerpentTail;
            break;
        default:
            break;
    }

    reg.add_component(part, position{x, y});
    reg.add_component(part, velocity{0.0f, 0.0f});
    reg.add_component(part, health{part_health, part_health});
    reg.add_component(part, collision_box{50.0f, 50.0f});
    reg.add_component(part, damage_on_contact{15, false});
    reg.add_component(part, entity_tag{entity_type});
    reg.add_component(part, enemy_tag{});
    reg.add_component(part, damage_flash_component{0.15f});

    serpent_part sp(type, index);
    sp.parent_entity = parent;
    sp.follow_delay = 0.05f + (index * 0.02f);
    reg.add_component(part, sp);

    reg.add_component(part, position_history{});

    switch (type) {
        case SerpentPartType::Head:
            controller.head_entity = part;
            break;
        case SerpentPartType::Body:
            controller.body_entities.push_back(part);
            break;
        case SerpentPartType::Tail:
            controller.tail_entity = part;
            break;
        default:
            break;
    }
}

void BossManager::spawn_serpent_scale_on_body(registry& reg, serpent_boss_controller& controller,
                                              entity body_entity, float x, float y,
                                              int scale_index) {
    entity scale = reg.spawn_entity();

    int scale_health = 150;

    reg.add_component(scale, position{x, y});
    reg.add_component(scale, velocity{0.0f, 0.0f});
    reg.add_component(scale, health{scale_health, scale_health});
    reg.add_component(scale, collision_box{40.0f, 40.0f});
    reg.add_component(scale, damage_on_contact{15, false});
    reg.add_component(scale, entity_tag{RType::EntityType::SerpentScale});
    reg.add_component(scale, enemy_tag{});
    reg.add_component(scale, damage_flash_component{0.15f});

    serpent_part sp(SerpentPartType::Scale, 100 + scale_index);
    sp.attached_body = body_entity;
    sp.can_attack = true;
    sp.attack_cooldown = 2.5f;
    reg.add_component(scale, sp);

    controller.scale_entities.push_back(scale);
}

void BossManager::update_serpent_spawn_animation(registry& reg, serpent_boss_controller& controller,
                                                 float dt) {
    controller.spawn_timer += dt;

    if (!controller.nest_visible && controller.spawn_timer >= 0.5f) {
        spawn_serpent_nest(reg, controller);
    }

    if (controller.spawn_timer >= controller.nest_rise_duration + 0.5f &&
        !controller.spawn_complete) {
        float serpent_progress = (controller.spawn_timer - controller.nest_rise_duration - 0.5f) /
                                 controller.serpent_emerge_duration;
        serpent_progress = std::min(1.0f, std::max(0.0f, serpent_progress));

        int total_chain_parts = 1 + controller.num_body_parts + 1;
        int parts_to_spawn = static_cast<int>(serpent_progress * total_chain_parts);

        float nest_x = 960.0f;
        float nest_y = 950.0f;

        while (controller.parts_spawned < parts_to_spawn &&
               controller.parts_spawned < total_chain_parts) {
            int index = controller.parts_spawned;
            float spawn_x = nest_x;
            float spawn_y = nest_y;

            std::optional<entity> parent = std::nullopt;

            if (index == 0) {
                spawn_serpent_part(reg, controller, SerpentPartType::Head, index, spawn_x,
                                   spawn_y - 50.0f, std::nullopt);
                parent = controller.head_entity;
            } else if (index <= controller.num_body_parts) {
                if (!controller.body_entities.empty()) {
                    for (int j = static_cast<int>(controller.body_entities.size()) - 1; j >= 0;
                         --j) {
                        auto& tag_opt = reg.get_component<entity_tag>(controller.body_entities[j]);
                        if (tag_opt.has_value() &&
                            tag_opt->type == RType::EntityType::SerpentBody) {
                            parent = controller.body_entities[j];
                            break;
                        }
                    }
                    if (!parent.has_value() && controller.head_entity.has_value()) {
                        parent = controller.head_entity;
                    }
                } else if (controller.head_entity.has_value()) {
                    parent = controller.head_entity;
                }

                spawn_serpent_part(reg, controller, SerpentPartType::Body, index, spawn_x, spawn_y,
                                   parent);

                int body_index = index;
                if (body_index % 3 == 0) {
                    entity last_body = controller.body_entities.back();
                    auto& body_pos = reg.get_component<position>(last_body);
                    if (body_pos.has_value()) {
                        spawn_serpent_scale_on_body(
                            reg, controller, last_body, body_pos->x, body_pos->y,
                            static_cast<int>(controller.scale_entities.size()));
                    }
                }
            } else {
                if (!controller.body_entities.empty()) {
                    for (int j = static_cast<int>(controller.body_entities.size()) - 1; j >= 0;
                         --j) {
                        auto& tag_opt = reg.get_component<entity_tag>(controller.body_entities[j]);
                        if (tag_opt.has_value() &&
                            tag_opt->type == RType::EntityType::SerpentBody) {
                            parent = controller.body_entities[j];
                            break;
                        }
                    }
                }
                spawn_serpent_part(reg, controller, SerpentPartType::Tail, index, spawn_x, spawn_y,
                                   parent);
            }

            controller.parts_spawned++;
        }

        if (controller.parts_spawned >= total_chain_parts) {
            controller.spawn_complete = true;
        }
    }
}

void BossManager::update_serpent_movement(registry& reg, serpent_boss_controller& controller,
                                          float dt) {
    if (!controller.spawn_complete || !controller.head_entity.has_value()) {
        return;
    }

    controller.movement_timer += dt;

    auto& head_pos = reg.get_component<position>(controller.head_entity.value());
    auto& head_vel = reg.get_component<velocity>(controller.head_entity.value());

    if (!head_pos.has_value() || !head_vel.has_value()) {
        return;
    }

    static const std::array<std::pair<float, float>, 4> WAYPOINTS = {
        {{300.0f, 200.0f}, {1620.0f, 200.0f}, {1620.0f, 800.0f}, {300.0f, 800.0f}}};

    float dx = controller.target_x - head_pos->x;
    float dy = controller.target_y - head_pos->y;
    float distance = std::sqrt(dx * dx + dy * dy);

    if (distance < 100.0f) {
        controller.previous_waypoint_idx = controller.current_waypoint_idx;

        int new_waypoint;
        do {
            std::uniform_int_distribution<int> dist(0, 3);
            new_waypoint = dist(rng_);
        } while (new_waypoint == controller.current_waypoint_idx ||
                 new_waypoint == controller.previous_waypoint_idx);

        controller.current_waypoint_idx = new_waypoint;
        controller.target_x = WAYPOINTS[static_cast<size_t>(controller.current_waypoint_idx)].first;
        controller.target_y =
            WAYPOINTS[static_cast<size_t>(controller.current_waypoint_idx)].second;

        dx = controller.target_x - head_pos->x;
        dy = controller.target_y - head_pos->y;
        distance = std::sqrt(dx * dx + dy * dy);
    }

    if (distance > 1.0f) {
        float speed = controller.movement_speed;

        float base_vx = (dx / distance) * speed;
        float base_vy = (dy / distance) * speed;

        float wave_offset = std::sin(controller.movement_timer * controller.wave_frequency) *
                            controller.wave_amplitude;
        float perp_x = -dy / distance;
        float perp_y = dx / distance;

        head_vel->vx = base_vx + perp_x * wave_offset * 0.2f;
        head_vel->vy = base_vy + perp_y * wave_offset * 0.2f;
    }

    auto& head_history = reg.get_component<position_history>(controller.head_entity.value());
    if (head_history.has_value()) {
        head_history->add_position(head_pos->x, head_pos->y);
    }
}

void BossManager::update_serpent_parts_follow(registry& reg, serpent_boss_controller& controller,
                                              float dt) {
    if (!controller.spawn_complete) {
        return;
    }

    const float segment_distance = 30.0f;

    std::optional<entity> prev_entity = controller.head_entity;

    for (size_t i = 0; i < controller.body_entities.size(); ++i) {
        entity part_ent = controller.body_entities[i];

        auto& part_pos = reg.get_component<position>(part_ent);
        auto& part_comp = reg.get_component<serpent_part>(part_ent);
        auto& part_history = reg.get_component<position_history>(part_ent);

        if (!part_pos.has_value() || !part_comp.has_value()) {
            continue;
        }

        if (prev_entity.has_value()) {
            auto& prev_pos = reg.get_component<position>(prev_entity.value());
            if (prev_pos.has_value()) {
                float dx = prev_pos->x - part_pos->x;
                float dy = prev_pos->y - part_pos->y;
                float distance = std::sqrt(dx * dx + dy * dy);

                if (distance > segment_distance) {
                    float target_x = prev_pos->x - (dx / distance) * segment_distance;
                    float target_y = prev_pos->y - (dy / distance) * segment_distance;

                    float follow_speed = 15.0f * dt;
                    part_pos->x += (target_x - part_pos->x) * follow_speed;
                    part_pos->y += (target_y - part_pos->y) * follow_speed;

                    if (distance > segment_distance * 3.0f) {
                        part_pos->x = target_x;
                        part_pos->y = target_y;
                    }
                }
            }
        }

        if (part_history.has_value()) {
            part_history->add_position(part_pos->x, part_pos->y);
        }

        prev_entity = part_ent;
    }

    for (entity scale_ent : controller.scale_entities) {
        auto& scale_pos = reg.get_component<position>(scale_ent);
        auto& scale_comp = reg.get_component<serpent_part>(scale_ent);

        if (!scale_pos.has_value() || !scale_comp.has_value()) {
            continue;
        }

        if (scale_comp->attached_body.has_value()) {
            try {
                auto& body_pos = reg.get_component<position>(scale_comp->attached_body.value());
                if (body_pos.has_value()) {
                    scale_pos->x = body_pos->x;
                    scale_pos->y = body_pos->y;
                }
            } catch (...) {}
        }
    }

    if (controller.tail_entity.has_value() && prev_entity.has_value()) {
        auto& tail_pos = reg.get_component<position>(controller.tail_entity.value());
        auto& prev_pos = reg.get_component<position>(prev_entity.value());

        if (tail_pos.has_value() && prev_pos.has_value()) {
            float dx = prev_pos->x - tail_pos->x;
            float dy = prev_pos->y - tail_pos->y;
            float distance = std::sqrt(dx * dx + dy * dy);

            if (distance > segment_distance) {
                float target_x = prev_pos->x - (dx / distance) * segment_distance;
                float target_y = prev_pos->y - (dy / distance) * segment_distance;

                float follow_speed = 15.0f * dt;
                tail_pos->x += (target_x - tail_pos->x) * follow_speed;
                tail_pos->y += (target_y - tail_pos->y) * follow_speed;

                if (distance > segment_distance * 3.0f) {
                    tail_pos->x = target_x;
                    tail_pos->y = target_y;
                }
            }
        }
    }
}

void BossManager::update_serpent_rotations(
    registry& reg, serpent_boss_controller& controller,
    const std::unordered_map<int, std::size_t>& client_entity_ids) {
    if (controller.head_entity.has_value()) {
        auto& head_vel = reg.get_component<velocity>(controller.head_entity.value());
        auto& head_part = reg.get_component<serpent_part>(controller.head_entity.value());

        if (head_vel.has_value() && head_part.has_value()) {
            if (std::abs(head_vel->vx) > 0.1f || std::abs(head_vel->vy) > 0.1f) {
                float angle = std::atan2(head_vel->vy, head_vel->vx) * 180.0f / 3.14159265f;
                head_part->rotation = angle;
            }
        }
    }

    for (size_t i = 0; i < controller.body_entities.size(); ++i) {
        entity part_ent = controller.body_entities[i];

        auto& part_pos = reg.get_component<position>(part_ent);
        auto& part_comp = reg.get_component<serpent_part>(part_ent);

        if (!part_pos.has_value() || !part_comp.has_value()) {
            continue;
        }

        std::optional<entity> next_entity;
        if (i == 0) {
            next_entity = controller.head_entity;
        } else {
            next_entity = controller.body_entities[i - 1];
        }

        if (next_entity.has_value()) {
            auto& next_pos = reg.get_component<position>(next_entity.value());
            if (next_pos.has_value()) {
                float dx = next_pos->x - part_pos->x;
                float dy = next_pos->y - part_pos->y;
                float angle = std::atan2(dy, dx) * 180.0f / 3.14159265f;
                part_comp->rotation = angle;
            }
        }
    }

    for (entity scale_ent : controller.scale_entities) {
        auto& scale_pos = reg.get_component<position>(scale_ent);
        auto& scale_comp = reg.get_component<serpent_part>(scale_ent);

        if (!scale_pos.has_value() || !scale_comp.has_value()) {
            continue;
        }

        float closest_dist = 999999.0f;
        float closest_px = 960.0f, closest_py = 540.0f;

        for (const auto& [client_id, entity_idx] : client_entity_ids) {
            auto player_ent = reg.entity_from_index(entity_idx);
            auto& player_pos = reg.get_component<position>(player_ent);
            if (player_pos.has_value()) {
                float dx = player_pos->x - scale_pos->x;
                float dy = player_pos->y - scale_pos->y;
                float dist = std::sqrt(dx * dx + dy * dy);
                if (dist < closest_dist) {
                    closest_dist = dist;
                    closest_px = player_pos->x;
                    closest_py = player_pos->y;
                }
            }
        }

        float dx = closest_px - scale_pos->x;
        float dy = closest_py - scale_pos->y;
        float angle = std::atan2(dy, dx) * 180.0f / 3.14159265f;
        scale_comp->rotation = angle - 90.0f;
    }

    if (controller.tail_entity.has_value()) {
        auto& tail_pos = reg.get_component<position>(controller.tail_entity.value());
        auto& tail_part = reg.get_component<serpent_part>(controller.tail_entity.value());

        if (tail_pos.has_value() && tail_part.has_value()) {
            std::optional<entity> last_body;
            if (!controller.body_entities.empty()) {
                last_body = controller.body_entities.back();
            }

            if (last_body.has_value()) {
                auto& body_pos = reg.get_component<position>(last_body.value());
                if (body_pos.has_value()) {
                    float dx = body_pos->x - tail_pos->x;
                    float dy = body_pos->y - tail_pos->y;
                    float angle = std::atan2(dy, dx) * 180.0f / 3.14159265f;
                    tail_part->rotation = angle;
                }
            }
        }
    }
}

void BossManager::handle_serpent_part_damage(registry& reg, serpent_boss_controller& controller,
                                             entity part_entity, int damage) {
    controller.take_global_damage(damage);
}

void BossManager::update_serpent_boss(registry& reg,
                                      std::optional<entity>& serpent_controller_entity,
                                      const std::unordered_map<int, std::size_t>& client_entity_ids,
                                      float dt) {
    if (!serpent_controller_entity.has_value()) {
        return;
    }

    auto& ctrl_opt = reg.get_component<serpent_boss_controller>(serpent_controller_entity.value());
    if (!ctrl_opt.has_value()) {
        return;
    }

    auto& controller = ctrl_opt.value();

    if (controller.is_defeated()) {
        auto& level_managers = reg.get_components<level_manager>();
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

        if (controller.nest_entity.has_value()) {
            try {
                reg.remove_component<entity_tag>(controller.nest_entity.value());
                reg.kill_entity(controller.nest_entity.value());
            } catch (...) {}
        }
        if (controller.head_entity.has_value()) {
            try {
                reg.remove_component<entity_tag>(controller.head_entity.value());
                reg.kill_entity(controller.head_entity.value());
            } catch (...) {}
        }
        for (auto& body_ent : controller.body_entities) {
            try {
                reg.remove_component<entity_tag>(body_ent);
                reg.kill_entity(body_ent);
            } catch (...) {}
        }
        for (auto& scale_ent : controller.scale_entities) {
            try {
                reg.remove_component<entity_tag>(scale_ent);
                reg.kill_entity(scale_ent);
            } catch (...) {}
        }
        if (controller.tail_entity.has_value()) {
            try {
                reg.remove_component<entity_tag>(controller.tail_entity.value());
                reg.kill_entity(controller.tail_entity.value());
            } catch (...) {}
        }

        try {
            reg.kill_entity(serpent_controller_entity.value());
        } catch (...) {}
        serpent_controller_entity = std::nullopt;
        return;
    }

    if (!controller.spawn_complete) {
        update_serpent_spawn_animation(reg, controller, dt);
        return;
    }

    update_serpent_movement(reg, controller, dt);
    update_serpent_parts_follow(reg, controller, dt);
    update_serpent_rotations(reg, controller, client_entity_ids);

    if (controller.head_entity.has_value()) {
        auto& head_health = reg.get_component<health>(controller.head_entity.value());
        if (head_health.has_value()) {
            head_health->current = controller.current_health;
            head_health->maximum = controller.total_health;
        }
    }

    update_serpent_attacks(reg, controller, client_entity_ids, dt);
}

void BossManager::update_serpent_attacks(
    registry& reg, serpent_boss_controller& controller,
    const std::unordered_map<int, std::size_t>& client_entity_ids, float dt) {
    controller.scale_shoot_timer += dt;
    if (controller.scale_shoot_timer >= controller.scale_shoot_cooldown) {
        controller.scale_shoot_timer = 0.0f;
        serpent_scale_shoot(reg, controller, client_entity_ids);
    }

    if (!controller.scream_active) {
        controller.scream_timer += dt;
        if (controller.scream_timer >= controller.scream_cooldown) {
            controller.scream_timer = 0.0f;
            controller.scream_active = true;
            controller.scream_elapsed = 0.0f;
            serpent_scream_attack(reg, controller);
        }
    } else {
        controller.scream_elapsed += dt;

        if (controller.head_entity.has_value()) {
            auto& head_pos = reg.get_component<position>(controller.head_entity.value());
            if (head_pos.has_value()) {
                entity scream_effect = reg.spawn_entity();
                reg.add_component(scream_effect, position{head_pos->x, head_pos->y});
                reg.add_component(scream_effect, velocity{0.0f, 0.0f});
                reg.add_component(scream_effect, entity_tag{static_cast<RType::EntityType>(0x18)});
                reg.add_component(scream_effect, explosion_tag{0.05f});
            }
        }

        if (controller.scream_elapsed >= controller.scream_duration) {
            controller.scream_active = false;
        }
    }

    if (!controller.laser_charging && !controller.laser_firing) {
        controller.laser_timer += dt;
        if (controller.laser_timer >= controller.laser_cooldown) {
            controller.laser_timer = 0.0f;
            controller.laser_charging = true;
            controller.laser_elapsed = 0.0f;
        }
    }

    if (controller.laser_charging || controller.laser_firing) {
        serpent_laser_attack(reg, controller, client_entity_ids, dt);
    }
}

void BossManager::serpent_scale_shoot(
    registry& reg, serpent_boss_controller& controller,
    const std::unordered_map<int, std::size_t>& client_entity_ids) {
    if (controller.scale_entities.empty())
        return;

    size_t scale_idx =
        static_cast<size_t>(controller.current_scale_index) % controller.scale_entities.size();
    entity scale_ent = controller.scale_entities[scale_idx];

    controller.current_scale_index =
        (controller.current_scale_index + 1) % static_cast<int>(controller.scale_entities.size());

    auto& scale_pos = reg.get_component<position>(scale_ent);
    if (!scale_pos.has_value())
        return;

    float target_x = 400.0f;
    float target_y = 540.0f;

    for (const auto& [client_id, entity_id] : client_entity_ids) {
        auto player = reg.entity_from_index(entity_id);
        auto player_pos_opt = reg.get_component<position>(player);
        if (player_pos_opt.has_value()) {
            target_x = player_pos_opt->x;
            target_y = player_pos_opt->y;
            break;
        }
    }

    float sx = scale_pos->x;
    float sy = scale_pos->y;

    float dx = target_x - sx;
    float dy = target_y - sy;
    float dist = std::sqrt(dx * dx + dy * dy);
    if (dist < 1.0f)
        dist = 1.0f;

    float speed = 350.0f;
    float vx = (dx / dist) * speed;
    float vy = (dy / dist) * speed;

    entity projectile = reg.spawn_entity();
    reg.add_component(projectile, position{sx, sy});
    reg.add_component(projectile, velocity{vx, vy});
    reg.add_component(projectile, collision_box{15.0f, 15.0f});
    reg.add_component(projectile, damage_on_contact{10, true});
    reg.add_component(projectile, projectile_tag{});
    reg.add_component(projectile, enemy_tag{});
    reg.add_component(projectile, entity_tag{static_cast<RType::EntityType>(0x07)});
}

void BossManager::serpent_scream_attack(registry& reg, serpent_boss_controller& controller) {
    if (!controller.head_entity.has_value())
        return;

    auto& head_pos = reg.get_component<position>(controller.head_entity.value());
    if (!head_pos.has_value())
        return;

    int num_homings = 6 + (rng_() % 3);

    for (int i = 0; i < num_homings; ++i) {
        float spawn_x, spawn_y;
        int side = i % 4;

        switch (side) {
            case 0:
                spawn_x = 100.0f + static_cast<float>(rng_() % 1720);
                spawn_y = -50.0f;
                break;
            case 1:
                spawn_x = 100.0f + static_cast<float>(rng_() % 1720);
                spawn_y = 1130.0f;
                break;
            case 2:
                spawn_x = -50.0f;
                spawn_y = 100.0f + static_cast<float>(rng_() % 880);
                break;
            case 3:
            default:
                spawn_x = 1970.0f;
                spawn_y = 100.0f + static_cast<float>(rng_() % 880);
                break;
        }

        float target_x = 960.0f + static_cast<float>(rng_() % 200) - 100.0f;
        float target_y = 540.0f + static_cast<float>(rng_() % 200) - 100.0f;

        serpent_spawn_homing(reg, spawn_x, spawn_y, target_x, target_y);
    }
}

void BossManager::serpent_spawn_homing(registry& reg, float x, float y, float target_x,
                                       float target_y) {
    entity homing = reg.spawn_entity();

    float dx = target_x - x;
    float dy = target_y - y;
    float dist = std::sqrt(dx * dx + dy * dy);
    if (dist < 1.0f)
        dist = 1.0f;

    float speed = 150.0f;
    float vx = (dx / dist) * speed;
    float vy = (dy / dist) * speed;

    reg.add_component(homing, position{x, y});
    reg.add_component(homing, velocity{vx, vy});
    reg.add_component(homing, health{10, 10});
    reg.add_component(homing, collision_box{40.0f, 40.0f});
    reg.add_component(homing, damage_on_contact{15, true});
    reg.add_component(homing, homing_component{180.0f, 2.0f});
    reg.add_component(homing, entity_tag{static_cast<RType::EntityType>(0x15)});
    reg.add_component(homing, enemy_tag{});
}

void BossManager::serpent_laser_attack(
    registry& reg, serpent_boss_controller& controller,
    const std::unordered_map<int, std::size_t>& client_entity_ids, float dt) {
    if (!controller.tail_entity.has_value())
        return;

    auto& tail_pos = reg.get_component<position>(controller.tail_entity.value());
    if (!tail_pos.has_value())
        return;

    if (controller.laser_charging) {
        controller.laser_elapsed += dt;

        float charge_progress = controller.laser_elapsed / controller.laser_charge_duration;
        entity charge_effect = reg.spawn_entity();
        reg.add_component(charge_effect, position{tail_pos->x, tail_pos->y});
        reg.add_component(charge_effect, velocity{0.0f, charge_progress * 100.0f});
        reg.add_component(charge_effect, entity_tag{static_cast<RType::EntityType>(0x19)});
        reg.add_component(charge_effect, explosion_tag{0.05f});

        if (controller.laser_elapsed >= controller.laser_charge_duration) {
            controller.laser_charging = false;
            controller.laser_firing = true;
            controller.laser_elapsed = 0.0f;

            float target_x = 400.0f;
            float target_y = 540.0f;
            for (const auto& [client_id, entity_id] : client_entity_ids) {
                auto player = reg.entity_from_index(entity_id);
                auto player_pos_opt = reg.get_component<position>(player);
                if (player_pos_opt.has_value()) {
                    target_x = player_pos_opt->x;
                    target_y = player_pos_opt->y;
                    break;
                }
            }

            float dx = target_x - tail_pos->x;
            float dy = target_y - tail_pos->y;
            float angle_to_player = std::atan2(dy, dx);

            controller.laser_sweep_direction = (rng_() % 2 == 0) ? 1 : -1;
            float offset = 1.0f * controller.laser_sweep_direction;
            controller.laser_start_angle = angle_to_player - offset;
            controller.laser_angle = controller.laser_start_angle;
        }
        return;
    }

    if (controller.laser_firing) {
        controller.laser_elapsed += dt;

        float sweep_speed = 0.6f;
        controller.laser_angle += sweep_speed * controller.laser_sweep_direction * dt;

        float laser_length = 1800.0f;
        float segment_spacing = 60.0f;
        int num_segments = static_cast<int>(laser_length / segment_spacing);

        float cos_angle = std::cos(controller.laser_angle);
        float sin_angle = std::sin(controller.laser_angle);

        entity laser_origin = reg.spawn_entity();
        reg.add_component(laser_origin, position{tail_pos->x, tail_pos->y});
        reg.add_component(laser_origin, velocity{controller.laser_angle * 100.0f, laser_length});
        reg.add_component(laser_origin, collision_box{50.0f, 50.0f});
        reg.add_component(laser_origin, damage_on_contact{10, false});
        reg.add_component(laser_origin, projectile_tag{});
        reg.add_component(laser_origin, enemy_tag{});
        reg.add_component(laser_origin, entity_tag{static_cast<RType::EntityType>(0x16)});
        reg.add_component(laser_origin, explosion_tag{0.05f});

        for (int i = 2; i <= num_segments; ++i) {
            float dist = static_cast<float>(i) * segment_spacing;
            float seg_x = tail_pos->x + cos_angle * dist;
            float seg_y = tail_pos->y + sin_angle * dist;
            if (seg_x < -100 || seg_x > 2020 || seg_y < -100 || seg_y > 1200)
                continue;

            // Create laser segment for collision
            entity laser_seg = reg.spawn_entity();
            reg.add_component(laser_seg, position{seg_x, seg_y});
            reg.add_component(laser_seg, velocity{0.0f, 0.0f});  // Stationary
            reg.add_component(laser_seg, collision_box{40.0f, 40.0f});
            reg.add_component(laser_seg, damage_on_contact{10, false});  // Reduced damage
            reg.add_component(laser_seg, projectile_tag{});
            reg.add_component(laser_seg, enemy_tag{});
            // Use same laser type - segments won't render as beam, just for collision
            reg.add_component(laser_seg, entity_tag{static_cast<RType::EntityType>(
                                             0x17)});            // Hidden collision type
            reg.add_component(laser_seg, explosion_tag{0.05f});  // Auto-destroy after 0.05s
        }

        // End laser after duration
        if (controller.laser_elapsed >= controller.laser_fire_duration) {
            controller.laser_firing = false;
            std::cout << "[BOSS] Serpent laser attack finished." << std::endl;
        }
    }
}

std::pair<int, int> BossManager::get_boss_health(registry& reg, std::optional<entity>& boss_entity,
                                                 std::optional<entity>& serpent_controller) {
    // Check Level 5 boss
    if (boss_entity.has_value()) {
        auto& health_opt = reg.get_component<health>(boss_entity.value());
        if (health_opt.has_value()) {
            return {health_opt->current, health_opt->maximum};
        }
    }

    // Check Level 10 serpent boss
    if (serpent_controller.has_value()) {
        auto& ctrl_opt = reg.get_component<serpent_boss_controller>(serpent_controller.value());
        if (ctrl_opt.has_value()) {
            return {ctrl_opt->current_health, ctrl_opt->total_health};
        }
    }

    return {0, 0};
}

}  // namespace server
