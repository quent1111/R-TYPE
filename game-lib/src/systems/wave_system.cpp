#include "systems/wave_system.hpp"
#include "components/game_components.hpp"
#include "entities/enemy_factory.hpp"

void waveSystem(registry& reg, float dt) {
    auto& wave_managers = reg.get_components<wave_manager>();

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
            
            manager.timer += dt;
            if (manager.timer >= manager.spawn_interval) {
                manager.timer = 0.0f;
                spawnEnemyWave(reg, manager.enemies_per_wave);
            }
        }
    }
}
