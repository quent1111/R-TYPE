#include "entities/projectile_factory.hpp"
#include "ecs/components.hpp"
#include "components/game_components.hpp"
#include <cmath>

entity createProjectile(registry& reg, float x, float y, float vx, float vy, int damage, 
                        WeaponUpgradeType upgrade_type, bool power_cannon_active) {
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

    std::string texture_path = "assets/r-typesheet1.png";
    int texture_x = 231;
    int texture_y = 102;
    int texture_w = 16;
    int texture_h = 17;
    float scale = 2.0f;
    float collision_w = 24.0f;
    float collision_h = 24.0f;
    float final_vx = vx;
    float final_vy = vy;

    if (power_cannon_active) {
        final_vx = vx * 2.0f;
        final_vy = vy * 2.0f;
    }

    float projectile_speed = std::sqrt(final_vx * final_vx + final_vy * final_vy);

    if (upgrade_type == WeaponUpgradeType::AllyMissile) {
        texture_path = "assets/missile.png";
        texture_x = 0;
        texture_y = 0;
        texture_w = 18;
        texture_h = 17;
        projectile_frames = {
            {0, 0, 18, 17}
        };
        scale = 2.5f;
        collision_w = 45.0f;
        collision_h = 42.0f;
    } else if (projectile_speed > 600.0f) {
        texture_path = "assets/canonpowerup.png";
        texture_x = 0;
        texture_y = 0;
        texture_w = 51;
        texture_h = 21;
        projectile_frames = {
            {0, 0, 51, 21},
            {52, 0, 51, 21}
        };
        scale = 2.0f;
        collision_w = 102.0f;
        collision_h = 42.0f;
    } else if (upgrade_type == WeaponUpgradeType::PowerShot) {
        scale = 3.5f;
        collision_w = 42.0f;
        collision_h = 42.0f;
        projectile_frames = {
            {264, 102, 16, 17},
            {280, 102, 16, 17}
        };
    }

    reg.add_component(projectile, position{x, y});
    reg.add_component(projectile, velocity{final_vx, final_vy});
    reg.add_component(projectile,
                      sprite_component{texture_path, texture_x, texture_y, texture_w, texture_h, scale});
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

entity createEnemy2Projectile(registry& reg, float x, float y, float vx, float vy, int damage) {
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

    std::vector<sf::IntRect> enemy2_projectile_frames = {
        {0, 0, 18, 19},
        {18, 0, 18, 19}
    };

    float scale = 2.0f;
    float collision_w = 30.0f;
    float collision_h = 30.0f;

    reg.add_component(projectile, position{x, y});
    reg.add_component(projectile, velocity{vx, vy});
    reg.add_component(projectile,
                      sprite_component{"assets/ennemi-projectile.png", 0, 0, 18, 19, scale});
    reg.add_component(projectile,
                      animation_component{enemy2_projectile_frames, 0.1f, true});
    reg.add_component(projectile, collision_box{collision_w, collision_h});
    reg.add_component(projectile, damage_on_contact{damage, true});
    reg.add_component(projectile, projectile_tag{});
    reg.add_component(projectile, entity_tag{RType::EntityType::Projectile});
    reg.add_component(projectile, enemy_tag{});

    return projectile;
}

entity createEnemy3Projectile(registry& reg, float x, float y, float vx, float vy, int damage, int projectile_type) {
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

    std::vector<sf::IntRect> projectile_frames = {
        {48, 0, 16, 14},
        {32, 0, 16, 14},
        {16, 0, 16, 14},
        {0, 0, 16, 14}
    };
    int texture_x = 0;
    int texture_y = 0;
    int texture_w = 16;
    int texture_h = 14;
    float scale = 2.5f;
    float collision_w = 35.0f;
    float collision_h = 35.0f;

    reg.add_component(projectile, position{x, y});
    reg.add_component(projectile, velocity{vx, vy});
    reg.add_component(projectile,
                      sprite_component{"assets/r-typesheet14-22.gif", texture_x, texture_y, texture_w, texture_h, scale});
    reg.add_component(projectile,
                      animation_component{projectile_frames, 0.2f, false});
    reg.add_component(projectile, collision_box{collision_w, collision_h});
    reg.add_component(projectile, damage_on_contact{damage, true});
    reg.add_component(projectile, projectile_tag{});
    reg.add_component(projectile, entity_tag{RType::EntityType::Projectile});
    reg.add_component(projectile, enemy_tag{});

    return projectile;
}

entity createEnemy4Projectile(registry& reg, float x, float y, float vx, float vy, int damage) {
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

    std::vector<sf::IntRect> projectile_frames = {
        {0, 0, 65, 18},
        {65, 0, 65, 18}
    };
    int texture_x = 0;
    int texture_y = 0;
    int texture_w = 65;
    int texture_h = 18;
    float scale = -2.0f;
    float collision_w = 40.0f;
    float collision_h = 30.0f;

    reg.add_component(projectile, position{x, y});
    reg.add_component(projectile, velocity{vx, vy});
    reg.add_component(projectile,
                      sprite_component{"assets/r-typesheet9-22.gif", texture_x, texture_y, texture_w, texture_h, scale});
    reg.add_component(projectile,
                      animation_component{projectile_frames, 0.2f, true});
    reg.add_component(projectile, collision_box{collision_w, collision_h});
    reg.add_component(projectile, damage_on_contact{damage, true});
    reg.add_component(projectile, projectile_tag{});
    reg.add_component(projectile, entity_tag{RType::EntityType::Projectile});
    reg.add_component(projectile, enemy_tag{});

    return projectile;
}

entity createEnemy5Projectile(registry& reg, float x, float y, float vx, float vy, int damage) {
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

    std::vector<sf::IntRect> projectile_frames = {
        {0, 0, 30, 12},
        {30, 0, 30, 12}
    };
    int texture_x = 0;
    int texture_y = 0;
    int texture_w = 30;
    int texture_h = 12;
    float scale = -3.0f;
    float collision_w = 45.0f;
    float collision_h = 20.0f;

    reg.add_component(projectile, position{x, y});
    reg.add_component(projectile, velocity{vx, vy});
    reg.add_component(projectile,
                      sprite_component{"assets/r-typesheet9-3.gif", texture_x, texture_y, texture_w, texture_h, scale});
    reg.add_component(projectile,
                      animation_component{projectile_frames, 0.1f, true});
    reg.add_component(projectile, collision_box{collision_w, collision_h});
    reg.add_component(projectile, damage_on_contact{damage, true});
    reg.add_component(projectile, projectile_tag{});
    reg.add_component(projectile, entity_tag{RType::EntityType::Projectile});
    reg.add_component(projectile, enemy_tag{});

    return projectile;
}
