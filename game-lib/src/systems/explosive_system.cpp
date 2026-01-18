#include "systems/explosive_system.hpp"
#include "components/logic_components.hpp"
#include "components/game_components.hpp"
#include "entities/explosion_factory.hpp"
#include <cmath>
#include <iostream>

void explosiveProjectileSystem(registry& reg, float dt) {
    auto& positions = reg.get_components<position>();
    auto& explosives = reg.get_components<explosive_projectile>();
    auto& healths = reg.get_components<health>();
    auto& player_tags = reg.get_components<player_tag>();
    auto& shields = reg.get_components<shield>();

    for (std::size_t i = 0; i < explosives.size(); ++i) {
        if (!explosives[i].has_value()) continue;
        if (!positions[i].has_value()) continue;

        auto& explosive = explosives[i].value();
        auto& pos = positions[i].value();

        explosive.update(dt);

        if (explosive.should_explode()) {
            explosive.has_exploded = true;

            std::cout << "[EXPLOSION] Grenade exploding at (" << pos.x << ", " << pos.y 
                      << ") - Radius: " << explosive.explosion_radius 
                      << " - Damage: " << explosive.explosion_damage << std::endl;

            for (int e = 0; e < 12; ++e) {
                float angle = (3.14159f * 2.0f * static_cast<float>(e)) / 12.0f;
                float offset_x = std::cos(angle) * 20.0f;
                float offset_y = std::sin(angle) * 20.0f;
                createExplosion(reg, pos.x + offset_x, pos.y + offset_y);
            }

            for (std::size_t p = 0; p < player_tags.size(); ++p) {
                if (!player_tags[p].has_value()) continue;
                if (!positions[p].has_value()) continue;
                if (!healths[p].has_value()) continue;

                auto& player_pos = positions[p].value();
                auto& player_health = healths[p].value();

                float dx = player_pos.x - pos.x;
                float dy = player_pos.y - pos.y;
                float distance = std::sqrt(dx * dx + dy * dy);

                if (distance <= explosive.explosion_radius) {
                    bool protected_by_shield = false;
                    if (p < shields.size() && shields[p].has_value()) {
                        auto& shield = shields[p].value();
                        if (shield.is_active()) {
                            protected_by_shield = true;
                            std::cout << "[EXPLOSION] Player protected by shield!" << std::endl;
                        }
                    }

                    if (!protected_by_shield) {
                        float damage_multiplier = 1.0f - (distance / explosive.explosion_radius);
                        int actual_damage = static_cast<int>(static_cast<float>(explosive.explosion_damage) * damage_multiplier);
                        if (actual_damage > 0) {
                            player_health.current -= actual_damage;
                            if (player_health.current < 0) player_health.current = 0;

                            std::cout << "[EXPLOSION] Player took " << actual_damage
                                      << " damage from explosion (distance: " << distance << ")" << std::endl;
                        }
                    }
                }
            }

            auto grenade_entity = reg.entity_from_index(i);
            reg.kill_entity(grenade_entity);
        }
    }
}
