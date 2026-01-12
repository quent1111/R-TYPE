#include "systems/wave_system.hpp"
#include "components/game_components.hpp"
#include "entities/enemy_factory.hpp"

void waveSystem(registry& reg, float dt) {
    auto& wave_managers = reg.get_components<wave_manager>();
    auto& level_managers = reg.get_components<level_manager>();

    for (size_t i = 0; i < level_managers.size(); ++i) {
        if (level_managers[i].has_value()) {
            auto& lvl_mgr = level_managers[i].value();
            lvl_mgr.update_intro_timer(dt);

            if (lvl_mgr.is_level_intro_active()) {
                return;
            }

            if (lvl_mgr.awaiting_upgrade_choice) {
                return;
            }
        }
    }

    bool has_manager = false;
    for (size_t i = 0; i < wave_managers.size(); ++i) {
        if (wave_managers[i].has_value()) {
            has_manager = true;
            break;
        }
    }

    if (!has_manager) {
        auto entity = reg.spawn_entity();
        reg.emplace_component<wave_manager>(entity, 3.0f, 3);
        return;
    }

    for (size_t i = 0; i < wave_managers.size(); ++i) {
        if (wave_managers[i]) {
            auto& manager = wave_managers[i].value();

            bool is_boss_level = false;
            for (size_t j = 0; j < level_managers.size(); ++j) {
                if (level_managers[j].has_value()) {
                    auto& lvl_mgr = level_managers[j].value();
                    int level = lvl_mgr.current_level;

                    if (level == 5 || level == 10 || level == 15) {
                        is_boss_level = true;
                        break;
                    }

                    manager.spawn_interval = 3.0f - static_cast<float>(level - 1) * 0.2f;
                    if (manager.spawn_interval < 1.0f) manager.spawn_interval = 1.0f;

                    manager.enemies_per_wave = 3 + (level - 1);
                    if (manager.enemies_per_wave > 8) manager.enemies_per_wave = 8;
                    break;
                }
            }

            if (!is_boss_level) {
                manager.timer += dt;
                if (manager.timer >= manager.spawn_interval) {
                    manager.timer = 0.0f;
                    spawnEnemyWave(reg, manager.enemies_per_wave);
                }
            }
        }
    }
}
