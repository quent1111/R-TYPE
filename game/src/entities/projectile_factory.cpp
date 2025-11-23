#include "entities/projectile_factory.hpp"
#include "ecs/components.hpp"
#include "components/game_components.hpp"

entity createProjectile(registry& reg, float x, float y, float vx, float vy, int damage) {
    entity projectile = reg.spawn_entity();

    reg.register_component<position>();
    reg.register_component<velocity>();
    reg.register_component<sprite_component>();
    reg.register_component<animation_component>();
    reg.register_component<collision_box>();
    reg.register_component<damage_on_contact>();
    reg.register_component<projectile_tag>();

    std::vector<sf::IntRect> projectile_frames = {
        {231, 102, 16, 17},
        {231 + 16, 102, 16, 17}
    };

    reg.add_component(projectile, position{x, y});
    reg.add_component(projectile, velocity{vx, vy});
    reg.add_component(projectile,
                      sprite_component{"assets/r-typesheet1.png", 231, 102, 16, 17, 2.0f});
    reg.add_component(projectile,
                      animation_component{projectile_frames, 0.08f, true});
    reg.add_component(projectile, collision_box{24.0f, 24.0f});
    reg.add_component(projectile, damage_on_contact{damage, true});
    reg.add_component(projectile, projectile_tag{});

    return projectile;
}
