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

entity createCustomProjectile(registry& reg, float x, float y, float vx, float vy, int damage,
                               const custom_attack_config& config) {
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

    // Build animation frames
    std::vector<sf::IntRect> custom_projectile_frames;
    for (int i = 0; i < config.projectile_frame_count; ++i) {
        custom_projectile_frames.push_back({
            i * config.projectile_frame_width, 0,
            config.projectile_frame_width, config.projectile_frame_height
        });
    }

    float collision_w = static_cast<float>(config.projectile_frame_width) * config.projectile_scale * 0.8f;
    float collision_h = static_cast<float>(config.projectile_frame_height) * config.projectile_scale * 0.8f;

    reg.add_component(projectile, position{x, y});
    reg.add_component(projectile, velocity{vx, vy});
    
    sprite_component sprite;
    sprite.texture_path = config.projectile_texture;
    sprite.texture_rect_x = 0;
    sprite.texture_rect_y = 0;
    sprite.texture_rect_w = config.projectile_frame_width;
    sprite.texture_rect_h = config.projectile_frame_height;
    sprite.scale = config.projectile_scale;
    sprite.mirror_x = config.projectile_mirror_x;
    sprite.mirror_y = config.projectile_mirror_y;
    sprite.rotation = config.projectile_rotation;
    reg.add_component(projectile, sprite);
    
    reg.add_component(projectile,
                      animation_component{custom_projectile_frames, config.projectile_frame_duration, true});
    reg.add_component(projectile, collision_box{collision_w, collision_h});
    reg.add_component(projectile, damage_on_contact{damage, true});
    reg.add_component(projectile, projectile_tag{});
    reg.add_component(projectile, entity_tag{RType::EntityType::CustomProjectile});
    reg.add_component(projectile, enemy_tag{});
    
    // Store the texture path as entity ID for identification
    reg.add_component(projectile, custom_entity_id{config.projectile_texture});

    return projectile;
}

entity createEnemy3Projectile(registry& reg, float x, float y, float vx, float vy, int damage, [[maybe_unused]] int projectile_type) {
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

entity createExplosiveGrenade(registry& reg, float x, float y, float vx, float vy, 
                              float lifetime, float explosion_radius, int explosion_damage) {
    entity grenade = reg.spawn_entity();

    reg.register_component<position>();
    reg.register_component<velocity>();
    reg.register_component<sprite_component>();
    reg.register_component<animation_component>();
    reg.register_component<collision_box>();
    reg.register_component<damage_on_contact>();
    reg.register_component<projectile_tag>();
    reg.register_component<entity_tag>();
    reg.register_component<enemy_tag>();
    reg.register_component<explosive_projectile>();

    std::vector<sf::IntRect> grenade_frames = {
        {0, 0, 32, 32},
        {33, 0, 32, 32},
        {66, 0, 32, 32},
        {33, 0, 32, 32}
    };

    int texture_x = 0;
    int texture_y = 0;
    int texture_w = 32;
    int texture_h = 32;
    float scale = 1.8f;
    float collision_w = 40.0f;
    float collision_h = 40.0f;

    reg.add_component(grenade, position{x, y});
    reg.add_component(grenade, velocity{vx, vy});
    reg.add_component(grenade,
                      sprite_component{"assets/r-typesheet16.gif", texture_x, texture_y, texture_w, texture_h, scale});
    reg.add_component(grenade,
                      animation_component{grenade_frames, 0.15f, true});
    reg.add_component(grenade, collision_box{collision_w, collision_h});
    reg.add_component(grenade, damage_on_contact{15, true});
    reg.add_component(grenade, projectile_tag{});
    reg.add_component(grenade, entity_tag{RType::EntityType::Projectile});
    reg.add_component(grenade, enemy_tag{});
    reg.add_component(grenade, explosive_projectile{lifetime, explosion_radius, explosion_damage});

    return grenade;
}
