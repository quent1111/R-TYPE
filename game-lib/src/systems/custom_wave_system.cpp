#include "systems/custom_wave_system.hpp"
#include "components/game_components.hpp"
#include "ecs/components.hpp"
#include <random>
#include <iostream>
#include <cmath>

void initCustomWaveState(custom_wave_state& state, const std::string& level_id) {
    state.active = true;
    state.level_id = level_id;
    state.current_wave_index = 0;
    state.wave_timer = 0.0f;
    state.spawn_timer = 0.0f;
    state.enemies_spawned_in_group = 0;
    state.current_enemy_group_index = 0;
    state.wave_in_progress = false;
    state.level_complete = false;
    state.total_enemies_alive = 0;
}

void resetCustomWaveState(custom_wave_state& state) {
    state.active = false;
    state.level_id.clear();
    state.current_wave_index = 0;
    state.wave_timer = 0.0f;
    state.spawn_timer = 0.0f;
    state.enemies_spawned_in_group = 0;
    state.current_enemy_group_index = 0;
    state.wave_in_progress = false;
    state.level_complete = false;
    state.total_enemies_alive = 0;
}

bool isCustomLevelComplete(const custom_wave_state& state) {
    return state.level_complete;
}

// Store custom enemy config for attack patterns
struct custom_enemy_config {
    std::string attack_type;          // "front", "targeted", "spread"
    int projectile_count = 1;
    float spread_angle = 30.0f;
    bool aim_at_player = false;
    rtype::level::SpriteConfig projectile_sprite;
};

void spawnCustomEnemy(registry& reg, const rtype::level::EnemyConfig& enemy_def,
                      const rtype::level::EnemySpawnConfig& spawn_config, float spawn_y) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    
    entity enemy = reg.spawn_entity();
    
    float spawn_x = 1950.0f + spawn_config.spawn_point.offset_x;
    float y_pos = spawn_y + spawn_config.spawn_point.offset_y;
    
    if (spawn_config.spawn_point.position_type == "absolute") {
        spawn_x = spawn_config.spawn_point.x;
        y_pos = spawn_config.spawn_point.y;
    }
    
    std::uniform_real_distribution<float> dis_y(100.0f, 900.0f);
    if (spawn_config.spawn_point.position_type == "screen_right") {
        spawn_x = 1950.0f + spawn_config.spawn_point.offset_x;
        if (spawn_config.spawn_point.offset_y == 0) {
            y_pos = dis_y(gen);
        } else {
            y_pos = 540.0f + spawn_config.spawn_point.offset_y;
        }
    }
    
    reg.add_component(enemy, position{spawn_x, y_pos});
    reg.add_component(enemy, velocity{-enemy_def.speed, 0.0f});
    reg.add_component(enemy, health{enemy_def.health});
    
    float fire_rate = enemy_def.attack.cooldown > 0.0f ? enemy_def.attack.cooldown : 2.0f;
    int proj_damage = enemy_def.attack.projectile.damage > 0 ? enemy_def.attack.projectile.damage : 15;
    float proj_speed = enemy_def.attack.projectile.speed > 0.0f ? enemy_def.attack.projectile.speed : 300.0f;
    reg.add_component(enemy, weapon{fire_rate, proj_speed, proj_damage});
    
    sprite_component sprite;
    sprite.texture_path = enemy_def.sprite.texture_path;
    sprite.texture_rect_x = 0;
    sprite.texture_rect_y = 0;
    sprite.texture_rect_w = enemy_def.sprite.frame_width;
    sprite.texture_rect_h = enemy_def.sprite.frame_height;
    sprite.scale = enemy_def.sprite.scale_x;
    // New: Apply mirror and rotation from config
    sprite.mirror_x = enemy_def.sprite.mirror_x;
    sprite.mirror_y = enemy_def.sprite.mirror_y;
    sprite.rotation = enemy_def.sprite.rotation;
    reg.add_component(enemy, sprite);
    
    animation_component anim;
    for (int f = 0; f < enemy_def.sprite.frame_count; ++f) {
        anim.frames.push_back(sf::IntRect(
            f * enemy_def.sprite.frame_width, 0,
            enemy_def.sprite.frame_width, enemy_def.sprite.frame_height));
    }
    anim.current_frame = 0;
    anim.frame_duration = enemy_def.sprite.frame_duration;
    anim.time_accumulator = 0.0f;
    anim.loop = true;
    reg.add_component(enemy, anim);
    
    float box_w = static_cast<float>(enemy_def.sprite.frame_width) * enemy_def.sprite.scale_x * 0.9f;
    float box_h = static_cast<float>(enemy_def.sprite.frame_height) * enemy_def.sprite.scale_y * 0.9f;
    reg.add_component(enemy, collision_box{box_w, box_h});
    reg.add_component(enemy, damage_on_contact{enemy_def.damage, false});
    reg.add_component(enemy, enemy_tag{});
    reg.add_component(enemy, entity_tag{RType::EntityType::CustomEnemy});  // Use CustomEnemy type
    
    // Store the custom entity ID for proper identification
    reg.add_component(enemy, custom_entity_id{enemy_def.id});
    
    // Store attack pattern info in a component for the firing system
    custom_attack_config attack_cfg;
    attack_cfg.pattern_type = enemy_def.attack.type;
    attack_cfg.projectile_count = enemy_def.attack.projectile_count;
    attack_cfg.spread_angle = enemy_def.attack.spread_angle;
    attack_cfg.aim_at_player = enemy_def.attack.aim_at_player;
    attack_cfg.projectile_texture = enemy_def.attack.projectile.sprite.texture_path;
    attack_cfg.projectile_frame_width = enemy_def.attack.projectile.sprite.frame_width;
    attack_cfg.projectile_frame_height = enemy_def.attack.projectile.sprite.frame_height;
    attack_cfg.projectile_frame_count = enemy_def.attack.projectile.sprite.frame_count;
    attack_cfg.projectile_frame_duration = enemy_def.attack.projectile.sprite.frame_duration;
    attack_cfg.projectile_scale = enemy_def.attack.projectile.sprite.scale_x;
    attack_cfg.projectile_mirror_x = enemy_def.attack.projectile.sprite.mirror_x;
    attack_cfg.projectile_mirror_y = enemy_def.attack.projectile.sprite.mirror_y;
    attack_cfg.projectile_rotation = enemy_def.attack.projectile.sprite.rotation;
    reg.add_component(enemy, attack_cfg);
    
    std::cout << "[CustomWave] Spawned custom enemy: " << enemy_def.name 
              << " (ID: " << enemy_def.id << ") at (" << spawn_x << ", " << y_pos << ")"
              << " attack_type: " << enemy_def.attack.type << std::endl;
}

void spawnCustomBoss(registry& reg, const rtype::level::EnemyConfig& boss_def) {
    entity boss = reg.spawn_entity();
    
    float spawn_x = 1950.0f;
    float spawn_y = 540.0f;
    
    reg.add_component(boss, position{spawn_x, spawn_y});
    reg.add_component(boss, velocity{0.0f, 0.0f});
    reg.add_component(boss, health{boss_def.health});
    
    float fire_rate = boss_def.attack.cooldown > 0.0f ? boss_def.attack.cooldown : 3.0f;
    int proj_damage = boss_def.attack.projectile.damage > 0 ? boss_def.attack.projectile.damage : 30;
    float proj_speed = boss_def.attack.projectile.speed > 0.0f ? boss_def.attack.projectile.speed : 400.0f;
    reg.add_component(boss, weapon{fire_rate, proj_speed, proj_damage});
    
    sprite_component sprite;
    sprite.texture_path = boss_def.sprite.texture_path;
    sprite.texture_rect_x = 0;
    sprite.texture_rect_y = 0;
    sprite.texture_rect_w = boss_def.sprite.frame_width;
    sprite.texture_rect_h = boss_def.sprite.frame_height;
    sprite.scale = boss_def.sprite.scale_x;
    sprite.mirror_x = boss_def.sprite.mirror_x;
    sprite.mirror_y = boss_def.sprite.mirror_y;
    sprite.rotation = boss_def.sprite.rotation;
    reg.add_component(boss, sprite);
    
    animation_component anim;
    for (int f = 0; f < boss_def.sprite.frame_count; ++f) {
        anim.frames.push_back(sf::IntRect(
            f * boss_def.sprite.frame_width, 0,
            boss_def.sprite.frame_width, boss_def.sprite.frame_height));
    }
    anim.current_frame = 0;
    anim.frame_duration = boss_def.sprite.frame_duration;
    anim.time_accumulator = 0.0f;
    anim.loop = true;
    reg.add_component(boss, anim);
    
    float box_w = static_cast<float>(boss_def.sprite.frame_width) * boss_def.sprite.scale_x * 0.8f;
    float box_h = static_cast<float>(boss_def.sprite.frame_height) * boss_def.sprite.scale_y * 0.8f;
    reg.add_component(boss, collision_box{box_w, box_h});
    reg.add_component(boss, damage_on_contact{boss_def.damage, false});
    reg.add_component(boss, enemy_tag{});
    reg.add_component(boss, entity_tag{RType::EntityType::CustomBoss});  // Use CustomBoss type
    reg.add_component(boss, boss_tag{});
    
    // Store the custom entity ID for proper identification
    reg.add_component(boss, custom_entity_id{boss_def.id});
    
    // Boss attack config
    custom_attack_config attack_cfg;
    attack_cfg.pattern_type = boss_def.attack.type;
    attack_cfg.projectile_count = boss_def.attack.projectile_count;
    attack_cfg.spread_angle = boss_def.attack.spread_angle;
    attack_cfg.aim_at_player = boss_def.attack.aim_at_player;
    attack_cfg.projectile_texture = boss_def.attack.projectile.sprite.texture_path;
    attack_cfg.projectile_frame_width = boss_def.attack.projectile.sprite.frame_width;
    attack_cfg.projectile_frame_height = boss_def.attack.projectile.sprite.frame_height;
    attack_cfg.projectile_frame_count = boss_def.attack.projectile.sprite.frame_count;
    attack_cfg.projectile_frame_duration = boss_def.attack.projectile.sprite.frame_duration;
    attack_cfg.projectile_scale = boss_def.attack.projectile.sprite.scale_x;
    attack_cfg.projectile_mirror_x = boss_def.attack.projectile.sprite.mirror_x;
    attack_cfg.projectile_mirror_y = boss_def.attack.projectile.sprite.mirror_y;
    attack_cfg.projectile_rotation = boss_def.attack.projectile.sprite.rotation;
    reg.add_component(boss, attack_cfg);
    
    std::cout << "[CustomWave] Spawned CUSTOM BOSS: " << boss_def.name 
              << " (ID: " << boss_def.id << ") with " << boss_def.health << " HP" << std::endl;
}

void updateCustomWaveSystem(registry& reg, custom_wave_state& state,
                            const rtype::level::LevelConfig& config, float dt) {
    if (!state.active || state.level_complete) {
        return;
    }
    
    auto& enemy_tags = reg.get_components<enemy_tag>();
    int alive_count = 0;
    for (size_t i = 0; i < enemy_tags.size(); ++i) {
        if (enemy_tags[i].has_value()) {
            alive_count++;
        }
    }
    state.total_enemies_alive = alive_count;
    
    if (state.current_wave_index >= static_cast<int>(config.waves.size())) {
        if (state.total_enemies_alive == 0) {
            state.level_complete = true;
            std::cout << "[CustomWave] Level complete!" << std::endl;
        }
        return;
    }
    
    const auto& current_wave = config.waves[static_cast<size_t>(state.current_wave_index)];
    
    if (!state.wave_in_progress) {
        state.wave_timer += dt;
        if (state.wave_timer >= current_wave.wave_delay) {
            state.wave_in_progress = true;
            state.wave_timer = 0.0f;
            state.spawn_timer = 0.0f;
            state.current_enemy_group_index = 0;
            state.enemies_spawned_in_group = 0;
            std::cout << "[CustomWave] Starting wave " << current_wave.wave_number 
                      << ": " << current_wave.name << std::endl;
        }
        return;
    }
    
    // Handle boss wave
    if (current_wave.is_boss_wave) {
        if (state.enemies_spawned_in_group == 0 && !current_wave.enemies.empty()) {
            const auto& boss_spawn = current_wave.enemies[0];
            auto it = config.enemy_definitions.find(boss_spawn.enemy_id);
            if (it != config.enemy_definitions.end()) {
                std::cout << "[CustomWave] Spawning boss: " << boss_spawn.enemy_id << std::endl;
                spawnCustomBoss(reg, it->second);
                state.enemies_spawned_in_group = 1;
            } else {
                std::cerr << "[CustomWave] ERROR: Boss " << boss_spawn.enemy_id << " not found!" << std::endl;
            }
        }
        // Boss wave complete when boss is dead
        if (state.enemies_spawned_in_group > 0 && state.total_enemies_alive == 0) {
            state.wave_in_progress = false;
            state.current_wave_index++;
            state.enemies_spawned_in_group = 0;
            std::cout << "[CustomWave] Boss defeated! Moving to next wave." << std::endl;
        }
        return;
    }
    
    if (state.current_enemy_group_index >= static_cast<int>(current_wave.enemies.size())) {
        if (state.total_enemies_alive == 0) {
            state.wave_in_progress = false;
            state.current_wave_index++;
            state.current_enemy_group_index = 0;
            state.enemies_spawned_in_group = 0;
            std::cout << "[CustomWave] Wave " << current_wave.wave_number << " cleared!" << std::endl;
        }
        return;
    }
    
    const auto& enemy_group = current_wave.enemies[static_cast<size_t>(state.current_enemy_group_index)];
    
    if (state.enemies_spawned_in_group < enemy_group.count) {
        state.spawn_timer += dt;
        if (state.spawn_timer >= enemy_group.spawn_delay) {
            state.spawn_timer = 0.0f;
            
            auto it = config.enemy_definitions.find(enemy_group.enemy_id);
            if (it != config.enemy_definitions.end()) {
                if (enemy_group.enemy_id.find("boss") != std::string::npos || 
                    enemy_group.enemy_id == "unicorn_boss") {
                    std::cerr << "[CustomWave] WARNING: Boss " << enemy_group.enemy_id 
                              << " in non-boss wave! Skipping." << std::endl;
                    state.enemies_spawned_in_group = enemy_group.count;
                } else {
                    static std::random_device rd;
                    static std::mt19937 gen(rd());
                    std::uniform_real_distribution<float> dis_y(100.0f, 900.0f);
                    float spawn_y = dis_y(gen);
                    
                    spawnCustomEnemy(reg, it->second, enemy_group, spawn_y);
                    state.enemies_spawned_in_group++;
                }
            } else {
                state.enemies_spawned_in_group = enemy_group.count;
            }
        }
    } else {
        state.current_enemy_group_index++;
        state.enemies_spawned_in_group = 0;
        state.spawn_timer = 0.0f;
    }
}

