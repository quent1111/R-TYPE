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
    reg.register_component<collision_box>();
    reg.register_component<damage_on_contact>();
    reg.register_component<enemy_tag>();

    reg.add_component(enemy, position{x, y});
    reg.add_component(enemy, velocity{-150.0f, 0.0f});
    reg.add_component(enemy, health{30});
    reg.add_component(enemy, sprite_component{"enemy.png", 48.0f, 48.0f, 255, 0, 0});
    reg.add_component(enemy, collision_box{40.0f, 40.0f});
    reg.add_component(enemy, damage_on_contact{25, false});
    reg.add_component(enemy, enemy_tag{});

    return enemy;
}

void spawnEnemyWave(registry& reg, int count) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dis_y(100.0f, 980.0f);

    for (int i = 0; i < count; ++i) {
        float spawn_x = 2000.0f;
        float spawn_y = dis_y(gen);
        createBasicEnemy(reg, spawn_x, spawn_y);
    }
}
