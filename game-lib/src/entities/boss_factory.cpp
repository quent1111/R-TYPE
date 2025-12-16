#include "entities/boss_factory.hpp"
#include "components/game_components.hpp"
#include "components/logic_components.hpp"
#include "../../../src/Common/Opcodes.hpp"

#include <iostream>
#include <SFML/Graphics.hpp>

entity createBoss(registry& reg, float x, float y, int boss_type) {
    std::cout << "[BOSS FACTORY] ===========================================" << std::endl;
    std::cout << "[BOSS FACTORY] Creating boss at position (" << x << ", " << y << ")" << std::endl;
    std::cout << "[BOSS FACTORY] Boss type: " << boss_type << std::endl;

    entity boss = reg.spawn_entity();
    std::cout << "[BOSS FACTORY] Boss entity spawned" << std::endl;

    reg.register_component<position>();
    reg.register_component<velocity>();
    reg.register_component<health>();
    reg.register_component<damage_on_contact>();
    reg.register_component<collision_box>();
    reg.register_component<boss_tag>();
    reg.register_component<entity_tag>();

    int boss_health = 1000;
    int contact_damage = 50;
    float boss_width = 300.0f;
    float boss_height = 300.0f;

    if (boss_type == 1) {
        boss_health = 1500;
        contact_damage = 60;
        boss_width = 400.0f;
        boss_height = 400.0f;
    }

    std::cout << "[BOSS FACTORY] Boss size: " << boss_width << "x" << boss_height << std::endl;
    std::cout << "[BOSS FACTORY] Boss health: " << boss_health << std::endl;

    reg.add_component(boss, position{x, y});
    
    reg.add_component(boss, velocity{-50.0f, 0.0f});
    
    reg.add_component(boss, health{boss_health, boss_health});
    
    reg.add_component(boss, damage_on_contact{contact_damage, false});
    
    reg.add_component(boss, collision_box{boss_width, boss_height, 0.0f, 0.0f});
    
    reg.add_component(boss, boss_tag{});
    reg.add_component(boss, entity_tag{RType::EntityType::Boss});

    std::cout << "[BOSS FACTORY] All components added successfully" << std::endl;
    std::cout << "[BOSS FACTORY] Boss creation complete!" << std::endl;
    std::cout << "[BOSS FACTORY] ===========================================" << std::endl;

    return boss;
}
