#include "entities/enemy_factory.hpp"
#include "ecs/components.hpp"
#include "components/game_components.hpp"
#include "components/logic_components.hpp"
#include <random>

static float get_difficulty_multiplier(registry& reg) {
    auto& settings = reg.get_components<game_settings>();
    for (const auto& setting : settings) {
        if (setting.has_value()) {
            return setting->difficulty_multiplier;
        }
    }
    return 1.0f;
}

static int get_current_level(registry& reg) {
    auto& level_managers = reg.get_components<level_manager>();
    for (const auto& lvl_mgr : level_managers) {
        if (lvl_mgr.has_value()) {
            return lvl_mgr->current_level;
        }
    }
    return 1;
}

static float get_level_scaling_multiplier(registry& reg) {
    int level = get_current_level(reg);
    if (level <= 1) return 1.0f;
    
    return 1.0f + (static_cast<float>(level - 1) * 0.1f);
}

entity createBasicEnemy(registry& reg, float x, float y) {
    entity enemy = reg.spawn_entity();

    reg.register_component<position>();
    reg.register_component<velocity>();
    reg.register_component<health>();
    reg.register_component<sprite_component>();
    reg.register_component<animation_component>();
    reg.register_component<collision_box>();
    reg.register_component<damage_on_contact>();
    reg.register_component<enemy_tag>();
    reg.register_component<entity_tag>();
    reg.register_component<weapon>();

    float difficulty_mult = get_difficulty_multiplier(reg);
    float level_mult = get_level_scaling_multiplier(reg);
    float total_mult = difficulty_mult * level_mult;
    
    int base_health = 10;
    int base_damage = 25;
    int scaled_health = static_cast<int>(static_cast<float>(base_health) * total_mult);
    int scaled_damage = static_cast<int>(static_cast<float>(base_damage) * (1.0f + (level_mult - 1.0f) * 0.5f));

    reg.add_component(enemy, position{x, y});
    reg.add_component(enemy, velocity{-150.0f, 0.0f});
    reg.add_component(enemy, health{scaled_health});

    reg.add_component(enemy, weapon{0.5f, 300.0f, 15});

    sprite_component sprite;
    sprite.texture_path = "assets/r-typesheet26.png";
    sprite.texture_rect_x = 0;
    sprite.texture_rect_y = 0;
    sprite.texture_rect_w = 65;
    sprite.texture_rect_h = 50;
    sprite.scale = 1.5f;
    reg.add_component(enemy, sprite);

    animation_component anim;
    anim.frames.push_back(sf::IntRect(0, 0, 65, 50));
    anim.frames.push_back(sf::IntRect(65, 0, 65, 50));
    anim.frames.push_back(sf::IntRect(130, 0, 65, 50));
    anim.current_frame = 0;
    anim.frame_duration = 0.15f;
    anim.time_accumulator = 0.0f;
    anim.loop = true;
    reg.add_component(enemy, anim);

    reg.add_component(enemy, collision_box{60.0f, 45.0f});
    reg.add_component(enemy, damage_on_contact{scaled_damage, false});
    reg.add_component(enemy, enemy_tag{});
    reg.add_component(enemy, entity_tag{RType::EntityType::Enemy});

    return enemy;
}

entity createSecondaryEnemy(registry& reg, float x, float y) {
    entity enemy = reg.spawn_entity();

    reg.register_component<position>();
    reg.register_component<velocity>();
    reg.register_component<health>();
    reg.register_component<sprite_component>();
    reg.register_component<animation_component>();
    reg.register_component<collision_box>();
    reg.register_component<damage_on_contact>();
    reg.register_component<enemy_tag>();
    reg.register_component<entity_tag>();
    reg.register_component<weapon>();

    float difficulty_mult = get_difficulty_multiplier(reg);
    float level_mult = get_level_scaling_multiplier(reg);
    float total_mult = difficulty_mult * level_mult;
    
    int base_health = 15;
    int base_damage = 30;
    int scaled_health = static_cast<int>(static_cast<float>(base_health) * total_mult);
    int scaled_damage = static_cast<int>(static_cast<float>(base_damage) * (1.0f + (level_mult - 1.0f) * 0.5f));

    reg.add_component(enemy, position{x, y});
    reg.add_component(enemy, velocity{-120.0f, 0.0f});
    reg.add_component(enemy, health{scaled_health});

    reg.add_component(enemy, weapon{0.7f, 250.0f, 20});

    sprite_component sprite;
    sprite.texture_path = "assets/r-typesheet24.png";
    sprite.texture_rect_x = 0;
    sprite.texture_rect_y = 0;
    sprite.texture_rect_w = 65;
    sprite.texture_rect_h = 66;
    sprite.scale = 1.5f;
    sprite.flip_horizontal = true;
    reg.add_component(enemy, sprite);

    animation_component anim;
    anim.frames.push_back(sf::IntRect(0, 0, 65, 66));
    anim.frames.push_back(sf::IntRect(65, 0, 65, 66));
    anim.frames.push_back(sf::IntRect(130, 0, 65, 66));
    anim.frames.push_back(sf::IntRect(195, 0, 65, 66));
    anim.frames.push_back(sf::IntRect(260, 0, 66, 66));
    anim.current_frame = 0;
    anim.frame_duration = 0.12f;
    anim.time_accumulator = 0.0f;
    anim.loop = true;
    reg.add_component(enemy, anim);

    reg.add_component(enemy, collision_box{60.0f, 60.0f});
    reg.add_component(enemy, damage_on_contact{scaled_damage, false});
    reg.add_component(enemy, enemy_tag{});
    reg.add_component(enemy, entity_tag{RType::EntityType::Enemy2});

    return enemy;
}

entity createFlyingEnemy(registry& reg, float x, float y) {
    entity enemy = reg.spawn_entity();

    float difficulty_mult = get_difficulty_multiplier(reg);
    float level_mult = get_level_scaling_multiplier(reg);
    float combined_mult = difficulty_mult * level_mult;
    int scaled_health = static_cast<int>(40.0f * combined_mult);
    int scaled_damage = static_cast<int>(50.0f * combined_mult);
    int scaled_weapon_damage = static_cast<int>(35.0f * combined_mult);

    reg.register_component<position>();
    reg.register_component<velocity>();
    reg.register_component<health>();
    reg.register_component<sprite_component>();
    reg.register_component<animation_component>();
    reg.register_component<collision_box>();
    reg.register_component<damage_on_contact>();
    reg.register_component<enemy_tag>();
    reg.register_component<entity_tag>();
    reg.register_component<weapon>();

    reg.add_component(enemy, position{x, y});
    reg.add_component(enemy, velocity{-250.0f, 0.0f});
    reg.add_component(enemy, health{scaled_health});

    reg.add_component(enemy, weapon{0.6f, 400.0f, scaled_weapon_damage});

    sprite_component sprite;
    sprite.texture_path = "assets/r-typesheet14-1.gif";
    sprite.texture_rect_x = 62;
    sprite.texture_rect_y = 0;
    sprite.texture_rect_w = 68;
    sprite.texture_rect_h = 52;
    sprite.scale = 1.8f;
    sprite.flip_horizontal = false;
    reg.add_component(enemy, sprite);

    animation_component anim;
    anim.frames.push_back(sf::IntRect(62, 0, 60, 52));
    anim.frames.push_back(sf::IntRect(3, 0, 58, 52));
    anim.frames.push_back(sf::IntRect(62, 0, 60, 52));
    anim.current_frame = 0;
    anim.frame_duration = 0.15f;
    anim.time_accumulator = 0.0f;
    anim.loop = true;
    reg.add_component(enemy, anim);

    reg.add_component(enemy, collision_box{70.0f, 80.0f});
    reg.add_component(enemy, damage_on_contact{scaled_damage, false});
    reg.add_component(enemy, enemy_tag{});
    reg.add_component(enemy, entity_tag{RType::EntityType::FlyingEnemy});

    return enemy;
}

entity createWaveEnemy(registry& reg, float x, float y) {
    entity enemy = reg.spawn_entity();

    float difficulty_mult = get_difficulty_multiplier(reg);
    float level_mult = get_level_scaling_multiplier(reg);
    float combined_mult = difficulty_mult * level_mult;
    int scaled_health = static_cast<int>(30.0f * combined_mult);
    int scaled_damage = static_cast<int>(35.0f * combined_mult);
    int scaled_weapon_damage = static_cast<int>(25.0f * combined_mult);

    reg.register_component<position>();
    reg.register_component<velocity>();
    reg.register_component<health>();
    reg.register_component<sprite_component>();
    reg.register_component<animation_component>();
    reg.register_component<collision_box>();
    reg.register_component<damage_on_contact>();
    reg.register_component<enemy_tag>();
    reg.register_component<entity_tag>();
    reg.register_component<weapon>();

    reg.add_component(enemy, position{x, y});
    reg.add_component(enemy, velocity{-200.0f, 0.0f});
    reg.add_component(enemy, health{scaled_health});

    reg.add_component(enemy, weapon{1.0f, 500.0f, scaled_weapon_damage});

    sprite_component sprite;
    sprite.texture_path = "assets/r-typesheet9-1.gif";
    sprite.texture_rect_x = 56;
    sprite.texture_rect_y = 0;
    sprite.texture_rect_w = 55;
    sprite.texture_rect_h = 59;
    sprite.scale = 1.5f;
    sprite.flip_horizontal = false;
    reg.add_component(enemy, sprite);

    animation_component anim;
    anim.frames.push_back(sf::IntRect(56, 0, 55, 59));
    anim.current_frame = 0;
    anim.frame_duration = 0.1f;
    anim.time_accumulator = 0.0f;
    anim.loop = true;
    reg.add_component(enemy, anim);

    reg.add_component(enemy, collision_box{55.0f, 60.0f});
    reg.add_component(enemy, damage_on_contact{scaled_damage, false});
    reg.add_component(enemy, enemy_tag{});
    reg.add_component(enemy, entity_tag{RType::EntityType::Enemy4});

    return enemy;
}

entity createTankEnemy(registry& reg, float x, float y) {
    entity enemy = reg.spawn_entity();

    float difficulty_mult = get_difficulty_multiplier(reg);
    float level_mult = get_level_scaling_multiplier(reg);
    float combined_mult = difficulty_mult * level_mult;
    int scaled_health = static_cast<int>(50.0f * combined_mult);
    int scaled_damage = static_cast<int>(40.0f * combined_mult);
    int scaled_weapon_damage = static_cast<int>(30.0f * combined_mult);

    reg.register_component<position>();
    reg.register_component<velocity>();
    reg.register_component<health>();
    reg.register_component<sprite_component>();
    reg.register_component<animation_component>();
    reg.register_component<collision_box>();
    reg.register_component<damage_on_contact>();
    reg.register_component<enemy_tag>();
    reg.register_component<entity_tag>();
    reg.register_component<weapon>();

    reg.add_component(enemy, position{x, y});
    reg.add_component(enemy, velocity{-150.0f, 0.0f});
    reg.add_component(enemy, health{scaled_health});

    reg.add_component(enemy, weapon{0.8f, 450.0f, scaled_weapon_damage});

    sprite_component sprite;
    sprite.texture_path = "assets/r-typesheet7.gif";
    sprite.texture_rect_x = 66;
    sprite.texture_rect_y = 34;
    sprite.texture_rect_w = 33;
    sprite.texture_rect_h = 33;
    sprite.scale = 2.5f;
    sprite.flip_horizontal = false;
    reg.add_component(enemy, sprite);

    animation_component anim;
    anim.frames.push_back(sf::IntRect(66, 34, 33, 33));
    anim.current_frame = 0;
    anim.frame_duration = 0.1f;
    anim.time_accumulator = 0.0f;
    anim.loop = true;
    reg.add_component(enemy, anim);

    reg.add_component(enemy, collision_box{60.0f, 60.0f});
    reg.add_component(enemy, damage_on_contact{scaled_damage, false});
    reg.add_component(enemy, enemy_tag{});
    reg.add_component(enemy, entity_tag{RType::EntityType::Enemy5});

    return enemy;
}

void spawnEnemyWave(registry& reg, int count, int level) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis_y(100.0f, 980.0f);
    std::uniform_real_distribution<float> dis_x(2000.0f, 2300.0f);
    std::uniform_real_distribution<float> dis_type(0.0f, 1.0f);

    for (int i = 0; i < count; ++i) {
        float spawn_x = dis_x(gen);
        float spawn_y = dis_y(gen);

        float type_roll = dis_type(gen);
        if (level >= 11) {
            if (type_roll < 0.33f) {
                createFlyingEnemy(reg, spawn_x, spawn_y);
            } else if (type_roll < 0.66f) {
                createWaveEnemy(reg, spawn_x, spawn_y);
            } else {
                createTankEnemy(reg, spawn_x, spawn_y);
            }
        }
        else {
            if (type_roll < 0.70f) {
                createBasicEnemy(reg, spawn_x, spawn_y);
            } else {
                createSecondaryEnemy(reg, spawn_x, spawn_y);
            }
        }
    }
}