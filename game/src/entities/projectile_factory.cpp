#include "entities/projectile_factory.hpp"
#include "ecs/components.hpp"
#include "components/game_components.hpp"

entity createProjectile(registry& reg, float x, float y, float vx, float vy, int damage) {
    entity projectile = reg.spawn_entity();

    reg.register_component<position>();
    reg.register_component<velocity>();
    reg.register_component<sprite_component>();
    reg.register_component<collision_box>();
    reg.register_component<damage_on_contact>();
    reg.register_component<projectile_tag>();

    reg.add_component(projectile, position{x, y});
    reg.add_component(projectile, velocity{vx, vy});
    reg.add_component(projectile, sprite_component{"projectile.png", 16.0f, 8.0f, 255, 255, 0});
    reg.add_component(projectile, collision_box{14.0f, 6.0f});
    reg.add_component(projectile, damage_on_contact{damage, true});
    reg.add_component(projectile, projectile_tag{});

    return projectile;
}
