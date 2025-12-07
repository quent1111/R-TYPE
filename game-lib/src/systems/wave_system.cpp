#include "systems/wave_system.hpp"
#include "components/game_components.hpp"
#include "entities/enemy_factory.hpp"

void waveSystem(registry& reg, float dt) {
    auto& wave_managers = reg.get_components<wave_manager>();
    auto& level_managers = reg.get_components<level_manager>();

    // Mettre à jour le timer d'intro du niveau
    for (size_t i = 0; i < level_managers.size(); ++i) {
        if (level_managers[i].has_value()) {
            auto& lvl_mgr = level_managers[i].value();
            lvl_mgr.update_intro_timer(dt);
            
            // Ne pas spawner pendant l'intro du niveau
            if (lvl_mgr.is_level_intro_active()) {
                return;
            }
            
            // Ne pas spawner pendant le choix d'upgrade
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

    // S'il n'y a pas de manager, on en crée un (entité singleton invisible)
    if (!has_manager) {
        auto entity = reg.spawn_entity();
        reg.emplace_component<wave_manager>(entity, 3.0f, 3); // Spawn toutes les 3s, 3 ennemis
        return; 
    }

    for (size_t i = 0; i < wave_managers.size(); ++i) {
        if (wave_managers[i]) {
            auto& manager = wave_managers[i].value();
            
            // Ajuster la difficulté selon le niveau
            for (size_t j = 0; j < level_managers.size(); ++j) {
                if (level_managers[j].has_value()) {
                    auto& lvl_mgr = level_managers[j].value();
                    int level = lvl_mgr.current_level;
                    
                    // Plus le niveau est élevé, plus le spawn est rapide et il y a d'ennemis
                    manager.spawn_interval = 3.0f - (level - 1) * 0.2f;
                    if (manager.spawn_interval < 1.0f) manager.spawn_interval = 1.0f;
                    
                    manager.enemies_per_wave = 3 + (level - 1);
                    if (manager.enemies_per_wave > 8) manager.enemies_per_wave = 8;
                    break;
                }
            }
            
            manager.timer += dt;
            if (manager.timer >= manager.spawn_interval) {
                manager.timer = 0.0f;
                spawnEnemyWave(reg, manager.enemies_per_wave);
            }
        }
    }
}
