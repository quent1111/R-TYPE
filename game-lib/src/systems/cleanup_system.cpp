#include "systems/cleanup_system.hpp"
#include "ecs/components.hpp"
#include "components/game_components.hpp"
#include "components/logic_components.hpp"
#include "entities/explosion_factory.hpp"
#include <vector>

void cleanupSystem(registry& reg, float dt) {
    auto& positions = reg.get_components<position>();
    auto& healths = reg.get_components<health>();
    auto& enemy_tags = reg.get_components<enemy_tag>();
    auto& player_tags = reg.get_components<player_tag>();
    auto& serpent_parts = reg.get_components<serpent_part>();
    auto& tags = reg.get_components<entity_tag>();

    std::vector<entity> entities_to_kill;

    for (std::size_t i = 0; i < healths.size(); ++i) {
        auto& health_opt = healths[i];

        if (health_opt && health_opt.value().is_dead()) {
            bool is_player = (i < player_tags.size() && player_tags[i]);
            if (is_player) {
                continue;
            }
            bool is_serpent_part = (i < serpent_parts.size() && serpent_parts[i].has_value());
            if (is_serpent_part) {
                continue;
            }
            bool is_compiler_part = false;
            if (i < tags.size() && tags[i].has_value()) {
                auto entity_type = tags[i]->type;
                is_compiler_part = (entity_type == RType::EntityType::CompilerPart1 ||
                                   entity_type == RType::EntityType::CompilerPart2 ||
                                   entity_type == RType::EntityType::CompilerPart3);
            }
            if (is_compiler_part) {
                continue;
            }
            bool is_boss = false;
            if (i < tags.size() && tags[i].has_value()) {
                auto entity_type = tags[i]->type;
                is_boss = (entity_type == RType::EntityType::Boss);
            }
            if (is_boss) {
                continue;
            }
            bool is_enemy = (i < enemy_tags.size() && enemy_tags[i]);
            if (is_enemy && i < positions.size() && positions[i]) {
                auto& pos = positions[i].value();
                createExplosion(reg, pos.x, pos.y);
            }
            entities_to_kill.push_back(reg.entity_from_index(i));
        }
    }

    auto& projectile_tags = reg.get_components<projectile_tag>();
    auto& explosion_tags = reg.get_components<explosion_tag>();

    for (std::size_t i = 0; i < positions.size(); ++i) {
        auto& pos_opt = positions[i];

        if (pos_opt) {
            auto& pos = pos_opt.value();
            bool is_serpent_part = (i < serpent_parts.size() && serpent_parts[i].has_value());
            if (is_serpent_part) {
                continue;
            }

            bool is_projectile = (i < projectile_tags.size() && projectile_tags[i]);
            bool is_enemy = (i < enemy_tags.size() && enemy_tags[i]);

            if (is_projectile || is_enemy) {
                if (pos.x < -200.0f || pos.x > 2200.0f || pos.y < -200.0f || pos.y > 1300.0f) {
                    entities_to_kill.push_back(reg.entity_from_index(i));
                }
            }
        }
    }

    for (std::size_t i = 0; i < explosion_tags.size(); ++i) {
        auto& explosion_opt = explosion_tags[i];
        if (explosion_opt) {
            auto& explosion = explosion_opt.value();
            explosion.elapsed += dt;
            if (explosion.elapsed >= explosion.lifetime) {
                entities_to_kill.push_back(reg.entity_from_index(i));
            }
        }
    }

    for (auto& ent : entities_to_kill) {
        reg.kill_entity(ent);
    }
}
