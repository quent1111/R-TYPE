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
    bool& boss_entrance_complete, float boss_target_x, float dt) {
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

    if (boss_animation_timer >= 9.2f && !boss_animation_complete) {
        boss_animation_complete = true;
        boss_shoot_timer = 0.0f;
        std::cout << "[BOSS] Animation complete! Boss will now start shooting!" << std::endl;
    }

    if (boss_animation_complete) {
        boss_shoot_timer += dt;

        if (boss_shoot_timer >= boss_shoot_cooldown) {
            boss_shoot_projectile(reg, boss_entity, client_entity_ids);
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

        reg.add_component(projectile, position{boss_x - 100.0f, boss_y});

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

}  // namespace server
