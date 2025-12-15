#include "entities/enemy_factory.hpp"
#include "ecs/components.hpp"
#include "components/game_components.hpp"
#include <random>

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

    reg.add_component(enemy, position{x, y});
    reg.add_component(enemy, velocity{-150.0f, 0.0f});
    reg.add_component(enemy, health{10});

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
    reg.add_component(enemy, damage_on_contact{25, false});
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

    reg.add_component(enemy, position{x, y});
    reg.add_component(enemy, velocity{-120.0f, 0.0f});  // Slightly slower
    reg.add_component(enemy, health{15});  // More HP

    reg.add_component(enemy, weapon{0.7f, 250.0f, 20});  // Slower fire rate

    sprite_component sprite;
    sprite.texture_path = "assets/r-typesheet24.png";
    sprite.texture_rect_x = 0;
    sprite.texture_rect_y = 0;
    sprite.texture_rect_w = 65;
    sprite.texture_rect_h = 66;
    sprite.scale = 1.5f;
    sprite.flip_horizontal = true;  // Flip to face left
    reg.add_component(enemy, sprite);

    animation_component anim;
    // 5 frames from r-typesheet24.gif (326x66 total, so 65.2x66 per frame)
    anim.frames.push_back(sf::IntRect(0, 0, 65, 66));
    anim.frames.push_back(sf::IntRect(65, 0, 65, 66));
    anim.frames.push_back(sf::IntRect(130, 0, 65, 66));
    anim.frames.push_back(sf::IntRect(195, 0, 65, 66));
    anim.frames.push_back(sf::IntRect(260, 0, 66, 66));  // Last frame slightly wider
    anim.current_frame = 0;
    anim.frame_duration = 0.12f;
    anim.time_accumulator = 0.0f;
    anim.loop = true;
    reg.add_component(enemy, anim);

    reg.add_component(enemy, collision_box{60.0f, 60.0f});
    reg.add_component(enemy, damage_on_contact{30, false});  // More damage
    reg.add_component(enemy, enemy_tag{});
    reg.add_component(enemy, entity_tag{RType::EntityType::Enemy2});  // Use Enemy2 type

    return enemy;
}

void spawnEnemyWave(registry& reg, int count) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis_y(100.0f, 980.0f);
    std::uniform_real_distribution<float> dis_x(2000.0f, 2300.0f);
    std::uniform_real_distribution<float> dis_type(0.0f, 1.0f);

    for (int i = 0; i < count; ++i) {
        float spawn_x = dis_x(gen);
        float spawn_y = dis_y(gen);
        
        // 70% chance for basic enemy (type 1), 30% for secondary enemy (type 2)
        float type_roll = dis_type(gen);
        if (type_roll < 0.70f) {
            createBasicEnemy(reg, spawn_x, spawn_y);
        } else {
            createSecondaryEnemy(reg, spawn_x, spawn_y);
        }
    }
}