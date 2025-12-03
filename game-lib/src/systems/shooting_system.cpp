#include "systems/shooting_system.hpp"
#include "ecs/components.hpp"
#include "components/game_components.hpp"
#include "entities/projectile_factory.hpp"

void shootingSystem(registry& reg, float dt) {
    auto& weapons = reg.get_components<weapon>();

    for (std::size_t i = 0; i < weapons.size(); ++i) {
        auto& weapon_opt = weapons[i];

        if (weapon_opt) {
            auto& wpn = weapon_opt.value();
            wpn.update(dt);
        }
    }
}
