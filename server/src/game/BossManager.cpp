#include "game/BossManager.hpp"

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

    std::cout << "[BOSS] Level 5 boss spawning from right side..." << std::endl;
    std::cout << "[BOSS] Boss will enter the screen and stop at X=" << boss_target_x << std::endl;
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
        std::cout << "[BOSS] Boss has been defeated! Removing boss entity..." << std::endl;
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
                std::cout << "[BOSS] Boss entrance complete! Stopped at X=" << boss_target_x
                          << std::endl;
            }
        }
        return;
    }

    boss_animation_timer += dt;

    if (boss_animation_timer >= 2.5f && !boss_animation_complete) {
        boss_animation_complete = true;
        boss_shoot_timer = 0.0f;
        std::cout << "[BOSS] Animation complete! Boss will now start shooting!" << std::endl;
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
        reg.add_component(projectile, damage_on_contact{50, false});

        reg.add_component(projectile, projectile_tag{});
        reg.add_component(projectile, enemy_tag{});

        reg.add_component(projectile, entity_tag{static_cast<RType::EntityType>(0x07)});

        std::cout << "[BOSS] Fired projectile 0x07 towards player at (" << target_x << ", "
                  << target_y << ")" << std::endl;
        std::cout << "[BOSS] Projectile velocity: (" << vx << ", " << vy << ")" << std::endl;
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
    reg.add_component(homing, damage_on_contact{30, false});

    reg.add_component(homing, homing_component{250.0f, 3.0f});

    reg.add_component(homing, entity_tag{RType::EntityType::HomingEnemy});

    std::cout << "[BOSS] Spawned homing enemy 0x09 at (" << boss_x - 100.0f << ", " << boss_y + 150.0f << ")" << std::endl;
}

void BossManager::update_homing_enemies(
    registry& reg, const std::unordered_map<int, std::size_t>& client_entity_ids, float dt) {

    auto& positions = reg.get_components<position>();
    auto& velocities = reg.get_components<velocity>();
    auto& homing_comps = reg.get_components<homing_component>();
    auto& entity_tags = reg.get_components<entity_tag>();
    for (std::size_t i = 0; i < entity_tags.size(); ++i) {
        if (!entity_tags[i].has_value() ||
            entity_tags[i]->type != RType::EntityType::HomingEnemy ||
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
            if (turn_factor > 1.0f) turn_factor = 1.0f;

            velocities[i]->vx += (desired_vx - velocities[i]->vx) * turn_factor;
            velocities[i]->vy += (desired_vy - velocities[i]->vy) * turn_factor;
        }
    }
}

}  // namespace server
