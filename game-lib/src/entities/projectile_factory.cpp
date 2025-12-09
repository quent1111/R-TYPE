#include "entities/projectile_factory.hpp"
#include "ecs/components.hpp"
#include "components/game_components.hpp"
#include <cmath>

entity createProjectile(registry& reg, float x, float y, float vx, float vy, int damage, 
                        WeaponUpgradeType upgrade_type) {
    entity projectile = reg.spawn_entity();

    reg.register_component<position>();
    reg.register_component<velocity>();
    reg.register_component<sprite_component>();
    reg.register_component<animation_component>();
    reg.register_component<collision_box>();
    reg.register_component<damage_on_contact>();
    reg.register_component<projectile_tag>();
    reg.register_component<entity_tag>();

    std::vector<sf::IntRect> projectile_frames = {
        {231, 102, 16, 17},
        {231 + 16, 102, 16, 17}
    };

    float scale = 2.0f;
    float collision_w = 24.0f;
    float collision_h = 24.0f;

    if (upgrade_type == WeaponUpgradeType::PowerShot) {
        scale = 3.5f;
        collision_w = 42.0f;
        collision_h = 42.0f;
        projectile_frames = {
            {264, 102, 16, 17},
            {280, 102, 16, 17}
        };
    }

    reg.add_component(projectile, position{x, y});
    reg.add_component(projectile, velocity{vx, vy});
    reg.add_component(projectile,
                      sprite_component{"assets/r-typesheet1.png", 231, 102, 16, 17, scale});
    reg.add_component(projectile,
                      animation_component{projectile_frames, 0.08f, true});
    reg.add_component(projectile, collision_box{collision_w, collision_h});
    reg.add_component(projectile, damage_on_contact{damage, true});
    reg.add_component(projectile, projectile_tag{});
    reg.add_component(projectile, entity_tag{RType::EntityType::Projectile});

    return projectile;
}

entity createEnemyProjectile(registry& reg, float x, float y, float vx, float vy, int damage) {
    entity projectile = reg.spawn_entity();

    reg.register_component<position>();
    reg.register_component<velocity>();
    reg.register_component<sprite_component>();
    reg.register_component<animation_component>();
    reg.register_component<collision_box>();
    reg.register_component<damage_on_contact>();
    reg.register_component<projectile_tag>();
    reg.register_component<entity_tag>();
    reg.register_component<enemy_tag>();

    std::vector<sf::IntRect> enemy_projectile_frames = {
        {248, 102, 15, 17},
        {263, 102, 15, 17}
    };

    float scale = 2.0f;
    float collision_w = 20.0f;
    float collision_h = 20.0f;

    reg.add_component(projectile, position{x, y});
    reg.add_component(projectile, velocity{vx, vy});
    reg.add_component(projectile,
                      sprite_component{"assets/r-typesheet1.png", 248, 102, 15, 17, scale});
    reg.add_component(projectile,
                      animation_component{enemy_projectile_frames, 0.08f, true});
    reg.add_component(projectile, collision_box{collision_w, collision_h});
    reg.add_component(projectile, damage_on_contact{damage, true});
    reg.add_component(projectile, projectile_tag{});
    reg.add_component(projectile, entity_tag{RType::EntityType::Projectile});
    reg.add_component(projectile, enemy_tag{});

    return projectile;
}
