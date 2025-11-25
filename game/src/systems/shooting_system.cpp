#include "systems/shooting_system.hpp"
#include "ecs/components.hpp"
#include "components/game_components.hpp"
#include "entities/projectile_factory.hpp"
#include <SFML/Window/Keyboard.hpp>

void shootingSystem(registry& reg, float dt) {
    auto& positions = reg.get_components<position>();
    auto& weapons = reg.get_components<weapon>();
    auto& player_tags = reg.get_components<player_tag>();

    for (std::size_t i = 0; i < positions.size() && i < weapons.size() &&
                            i < player_tags.size();
         ++i) {
        auto& pos_opt = positions[i];
        auto& weapon_opt = weapons[i];
        auto& player_opt = player_tags[i];

        if (pos_opt && weapon_opt && player_opt) {
            auto& pos = pos_opt.value();
            auto& wpn = weapon_opt.value();

            wpn.update(dt);
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Space) && wpn.can_shoot()) {
                float projectile_x = pos.x + 60.0f;
                float projectile_y = pos.y;
                createProjectile(reg, projectile_x, projectile_y, wpn.projectile_speed, 0.0f,
                                 wpn.damage);

                wpn.reset_shot_timer();
            }
        }
    }
}
