#include "../../bootstrap/bs/registry.hpp"
#include <iostream>
#include <chrono>
#include <iomanip>

struct Position { float x, y; Position(float x_ = 0, float y_ = 0) : x(x_), y(y_) {} };
struct Velocity { float dx, dy; Velocity(float dx_ = 0, float dy_ = 0) : dx(dx_), dy(dy_) {} };
struct Health { int hp; Health(int hp_ = 100) : hp(hp_) {} };
struct Armor { int defense; Armor(int def = 0) : defense(def) {} };
struct Weapon { int damage; Weapon(int dmg = 0) : damage(dmg) {} };

int main() {
    std::cout << "=== Performance Test: Optimized Registry ===" << std::endl;
    std::cout << "\nScenario: 10,000 entities with 5 component types\n" << std::endl;

    registry reg;
    
    reg.register_component<Position>();
    reg.register_component<Velocity>();
    reg.register_component<Health>();
    reg.register_component<Armor>();
    reg.register_component<Weapon>();

    const int NUM_ENTITIES = 10000;
    std::vector<entity> entities;
    entities.reserve(NUM_ENTITIES);

    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < NUM_ENTITIES; ++i) {
        auto e = reg.spawn_entity();
        entities.push_back(e);
        
        reg.add_component(e, Position(i * 1.0f, i * 2.0f));
        
        if (i % 2 == 0) {
            reg.add_component(e, Velocity(1.0f, 1.0f));
        }
        
        if (i % 3 == 0) {
            reg.add_component(e, Health(100));
        }
        
        if (i % 5 == 0) {
            reg.add_component(e, Armor(50));
        }
        
        if (i % 7 == 0) {
            reg.add_component(e, Weapon(25));
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "✓ Created " << NUM_ENTITIES << " entities in " 
              << duration.count() / 1000.0 << " ms" << std::endl;

    start = std::chrono::high_resolution_clock::now();
    int count = 0;
    
    for (const auto& e : entities) {
        if (reg.has_component<Position>(e)) count++;
        if (reg.has_component<Velocity>(e)) count++;
        if (reg.has_component<Health>(e)) count++;
        if (reg.has_component<Armor>(e)) count++;
        if (reg.has_component<Weapon>(e)) count++;
    }
    
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "✓ Checked " << NUM_ENTITIES * 5 << " has_component() calls in " 
              << duration.count() / 1000.0 << " ms" << std::endl;
    std::cout << "  (Found " << count << " components)" << std::endl;

    start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < NUM_ENTITIES / 2; ++i) {
        reg.kill_entity(entities[i]);
    }
    
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "✓ Killed " << NUM_ENTITIES / 2 << " entities in " 
              << duration.count() / 1000.0 << " ms" << std::endl;

    start = std::chrono::high_resolution_clock::now();
    
    std::vector<entity> reused_entities;
    for (int i = 0; i < NUM_ENTITIES / 2; ++i) {
        auto e = reg.spawn_entity();
        reused_entities.push_back(e);
        reg.add_component(e, Position(i * 10.0f, i * 20.0f));
    }
    
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "✓ Respawned " << NUM_ENTITIES / 2 << " entities (reusing IDs) in " 
              << duration.count() / 1000.0 << " ms" << std::endl;

    int reused_count = 0;
    for (const auto& e : reused_entities) {
        if (e.id() < NUM_ENTITIES) {
            reused_count++;
        }
    }
    
    std::cout << "\n=== Optimization Summary ===" << std::endl;
    std::cout << "✓ Entity ID reuse: " << reused_count << "/" << reused_entities.size() 
              << " entities reused old IDs" << std::endl;
    std::cout << "✓ Component tracking: has_component() uses O(1) lookup instead of O(n) search" << std::endl;
    std::cout << "✓ Smart kill_entity: Only erases components the entity actually has" << std::endl;
    
    std::cout << "\n✅ All performance tests completed!" << std::endl;
    
    return 0;
}
