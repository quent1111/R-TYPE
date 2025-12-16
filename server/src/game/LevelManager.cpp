#include "game/LevelManager.hpp"

namespace server {

std::optional<size_t> LevelManager::get_level_manager_index(registry& reg) {
    auto& level_managers = reg.get_components<level_manager>();
    for (size_t i = 0; i < level_managers.size(); ++i) {
        if (level_managers[i].has_value()) {
            return i;
        }
    }
    return std::nullopt;
}

bool LevelManager::check_level_completion(registry& reg) {
    auto& level_managers = reg.get_components<level_manager>();
    for (size_t i = 0; i < level_managers.size(); ++i) {
        if (level_managers[i].has_value()) {
            auto& lvl_mgr = level_managers[i].value();
            if (lvl_mgr.level_completed) {
                return true;
            }
        }
    }
    return false;
}

uint8_t LevelManager::get_current_level(registry& reg) {
    auto& level_managers = reg.get_components<level_manager>();
    for (size_t i = 0; i < level_managers.size(); ++i) {
        if (level_managers[i].has_value()) {
            return level_managers[i].value().current_level;
        }
    }
    return 1;
}

void LevelManager::advance_level(registry& reg) {
    auto& level_managers = reg.get_components<level_manager>();
    for (size_t i = 0; i < level_managers.size(); ++i) {
        if (level_managers[i].has_value()) {
            auto& lvl_mgr = level_managers[i].value();
            lvl_mgr.advance_to_next_level();
            std::cout << "[Game] Advanced to level " << lvl_mgr.current_level << std::endl;
            break;
        }
    }
}

void LevelManager::clear_enemies_and_projectiles(registry& reg,
                                                 std::optional<entity>& boss_entity) {
    auto& enemy_tags = reg.get_components<enemy_tag>();
    auto& projectile_tags = reg.get_components<projectile_tag>();
    auto& boss_tags = reg.get_components<boss_tag>();
    auto& homing_comps = reg.get_components<homing_component>();

    int enemies_cleared = 0;
    int projectiles_cleared = 0;
    int bosses_cleared = 0;
    int homing_cleared = 0;

    for (std::size_t i = 0; i < enemy_tags.size(); ++i) {
        if (enemy_tags[i].has_value()) {
            auto ent = reg.entity_from_index(i);
            reg.remove_component<entity_tag>(ent);
            reg.kill_entity(ent);
            enemies_cleared++;
        }
    }

    for (std::size_t i = 0; i < projectile_tags.size(); ++i) {
        if (projectile_tags[i].has_value()) {
            auto ent = reg.entity_from_index(i);
            reg.remove_component<entity_tag>(ent);
            reg.kill_entity(ent);
            projectiles_cleared++;
        }
    }

    for (std::size_t i = 0; i < boss_tags.size(); ++i) {
        if (boss_tags[i].has_value()) {
            auto ent = reg.entity_from_index(i);
            reg.remove_component<entity_tag>(ent);
            reg.kill_entity(ent);
            bosses_cleared++;
        }
    }

    for (std::size_t i = 0; i < homing_comps.size(); ++i) {
        if (homing_comps[i].has_value()) {
            auto ent = reg.entity_from_index(i);
            reg.remove_component<entity_tag>(ent);
            reg.kill_entity(ent);
            homing_cleared++;
        }
    }

    if (boss_entity.has_value()) {
        boss_entity = std::nullopt;
    }

    std::cout << "[Game] Cleared level: " << enemies_cleared << " enemies, " << projectiles_cleared
              << " projectiles, " << bosses_cleared << " bosses, " << homing_cleared
              << " homing enemies" << std::endl;
}

}  // namespace server
