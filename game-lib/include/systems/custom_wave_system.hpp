#pragma once

#include "../level/LevelConfig.hpp"
#include "ecs/registry.hpp"
#include <string>
#include <vector>
#include <optional>

struct custom_wave_state {
    bool active = false;
    std::string level_id;
    int current_wave_index = 0;
    float wave_timer = 0.0f;
    float spawn_timer = 0.0f;
    int enemies_spawned_in_group = 0;
    int current_enemy_group_index = 0;
    bool wave_in_progress = false;
    bool level_complete = false;
    int total_enemies_alive = 0;
};

void initCustomWaveState(custom_wave_state& state, const std::string& level_id);
void updateCustomWaveSystem(registry& reg, custom_wave_state& state, 
                            const rtype::level::LevelConfig& config, float dt);
void spawnCustomEnemy(registry& reg, const rtype::level::EnemyConfig& enemy_def,
                      const rtype::level::EnemySpawnConfig& spawn_config, float spawn_y);
void spawnCustomBoss(registry& reg, const rtype::level::EnemyConfig& boss_def, const rtype::level::EnemySpawnConfig& spawn_config);
bool isCustomLevelComplete(const custom_wave_state& state);
void resetCustomWaveState(custom_wave_state& state);

