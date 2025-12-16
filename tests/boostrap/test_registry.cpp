#include "../../bootstrap/bs/registry.hpp"
#include <iostream>
#include <cassert>
#include <string>

struct Position {
    float x, y;
    Position(float x_ = 0, float y_ = 0) : x(x_), y(y_) {}
    bool operator==(Position const& other) const { return x == other.x && y == other.y; }
};

struct Velocity {
    float dx, dy;
    Velocity(float dx_ = 0, float dy_ = 0) : dx(dx_), dy(dy_) {}
};

struct Health {
    int hp;
    Health(int hp_ = 100) : hp(hp_) {}
};

struct Name {
    std::string value;
    Name(std::string const& v = "") : value(v) {}
};

void test_spawn_entities() {
    std::cout << "=== Test spawn_entity ===" << std::endl;

    registry reg;

    auto entity0 = reg.spawn_entity();
    auto entity1 = reg.spawn_entity();
    auto entity2 = reg.spawn_entity();

    assert(static_cast<std::size_t>(entity0) == 0);
    assert(static_cast<std::size_t>(entity1) == 1);
    assert(static_cast<std::size_t>(entity2) == 2);

    std::cout << "✓ spawn_entity works (entities: " << static_cast<std::size_t>(entity0)
              << ", " << static_cast<std::size_t>(entity1) << ", " << static_cast<std::size_t>(entity2)
              << ")" << std::endl;
}

void test_register_and_get_components() {
    std::cout << "\n=== Test register_component & get_components ===" << std::endl;

    registry reg;

    auto& positions = reg.register_component<Position>();
    auto& velocities = reg.register_component<Velocity>();

    auto& positions2 = reg.get_components<Position>();
    auto& velocities2 = reg.get_components<Velocity>();

    assert(&positions == &positions2);
    assert(&velocities == &velocities2);

    std::cout << "✓ register_component works" << std::endl;
    std::cout << "✓ get_components returns same reference" << std::endl;
}

void test_add_component() {
    std::cout << "\n=== Test add_component ===" << std::endl;

    registry reg;

    auto player = reg.spawn_entity();
    auto enemy = reg.spawn_entity();

    Position pos1(10.0f, 20.0f);
    reg.add_component(player, pos1);

    reg.add_component(player, Velocity(1.0f, 2.0f));
    reg.add_component(enemy, Position(100.0f, 200.0f));
    reg.add_component(enemy, Health(50));

    auto& positions = reg.get_components<Position>();
    assert(positions[static_cast<std::size_t>(player)].has_value());
    assert(positions[static_cast<std::size_t>(player)]->x == 10.0f);
    assert(positions[static_cast<std::size_t>(enemy)].has_value());
    assert(positions[static_cast<std::size_t>(enemy)]->x == 100.0f);

    auto& velocities = reg.get_components<Velocity>();
    assert(velocities[static_cast<std::size_t>(player)].has_value());
    assert(velocities[static_cast<std::size_t>(player)]->dx == 1.0f);

    auto& healths = reg.get_components<Health>();
    assert(healths[static_cast<std::size_t>(enemy)].has_value());
    assert(healths[static_cast<std::size_t>(enemy)]->hp == 50);

    std::cout << "✓ add_component (lvalue) works" << std::endl;
    std::cout << "✓ add_component (rvalue) works" << std::endl;
}

void test_emplace_component() {
    std::cout << "\n=== Test emplace_component ===" << std::endl;

    registry reg;

    auto entity = reg.spawn_entity();

    reg.emplace_component<Position>(entity, 50.0f, 60.0f);
    reg.emplace_component<Name>(entity, "Player");

    auto& positions = reg.get_components<Position>();
    assert(positions[static_cast<std::size_t>(entity)].has_value());
    assert(positions[static_cast<std::size_t>(entity)]->x == 50.0f);
    assert(positions[static_cast<std::size_t>(entity)]->y == 60.0f);

    auto& names = reg.get_components<Name>();
    assert(names[static_cast<std::size_t>(entity)].has_value());
    assert(names[static_cast<std::size_t>(entity)]->value == "Player");

    std::cout << "✓ emplace_component works" << std::endl;
}

void test_has_component() {
    std::cout << "\n=== Test has_component ===" << std::endl;

    registry reg;

    auto entity = reg.spawn_entity();

    assert(!reg.has_component<Position>(entity));

    reg.add_component(entity, Position(1.0f, 2.0f));

    assert(reg.has_component<Position>(entity));
    assert(!reg.has_component<Velocity>(entity));

    std::cout << "✓ has_component works" << std::endl;
}

void test_remove_component() {
    std::cout << "\n=== Test remove_component ===" << std::endl;

    registry reg;

    auto entity = reg.spawn_entity();

    reg.add_component(entity, Position(10.0f, 20.0f));
    reg.add_component(entity, Velocity(1.0f, 2.0f));

    assert(reg.has_component<Position>(entity));
    assert(reg.has_component<Velocity>(entity));

    reg.remove_component<Position>(entity);

    assert(!reg.has_component<Position>(entity));
    assert(reg.has_component<Velocity>(entity));

    std::cout << "✓ remove_component works" << std::endl;
}

void test_kill_entity() {
    std::cout << "\n=== Test kill_entity ===" << std::endl;

    registry reg;

    auto entity = reg.spawn_entity();

    reg.add_component(entity, Position(10.0f, 20.0f));
    reg.add_component(entity, Velocity(1.0f, 2.0f));
    reg.add_component(entity, Health(100));

    assert(reg.has_component<Position>(entity));
    assert(reg.has_component<Velocity>(entity));
    assert(reg.has_component<Health>(entity));

    reg.kill_entity(entity);

    assert(!reg.has_component<Position>(entity));
    assert(!reg.has_component<Velocity>(entity));
    assert(!reg.has_component<Health>(entity));

    std::cout << "✓ kill_entity removes all components" << std::endl;
}

void test_ecs_system() {
    std::cout << "\n=== Test ECS System (Movement) ===" << std::endl;

    registry reg;

    auto player = reg.spawn_entity();
    auto enemy1 = reg.spawn_entity();
    auto enemy2 = reg.spawn_entity();
    auto static_obj = reg.spawn_entity();

    reg.emplace_component<Position>(player, 0.0f, 0.0f);
    reg.emplace_component<Velocity>(player, 5.0f, 3.0f);

    reg.emplace_component<Position>(enemy1, 100.0f, 100.0f);
    reg.emplace_component<Velocity>(enemy1, -2.0f, -1.0f);

    reg.emplace_component<Position>(enemy2, 200.0f, 200.0f);
    reg.emplace_component<Velocity>(enemy2, 1.0f, 1.0f);

    reg.emplace_component<Position>(static_obj, 50.0f, 50.0f);

    auto& positions = reg.get_components<Position>();
    auto& velocities = reg.get_components<Velocity>();

    for (std::size_t entity = 0; entity < positions.size(); ++entity) {
        if (positions[entity].has_value() && velocities[entity].has_value()) {
            positions[entity]->x += velocities[entity]->dx;
            positions[entity]->y += velocities[entity]->dy;
        }
    }

    assert(positions[static_cast<std::size_t>(player)]->x == 5.0f);
    assert(positions[static_cast<std::size_t>(player)]->y == 3.0f);

    assert(positions[static_cast<std::size_t>(enemy1)]->x == 98.0f);
    assert(positions[static_cast<std::size_t>(enemy1)]->y == 99.0f);

    assert(positions[static_cast<std::size_t>(enemy2)]->x == 201.0f);
    assert(positions[static_cast<std::size_t>(enemy2)]->y == 201.0f);

    assert(positions[static_cast<std::size_t>(static_obj)]->x == 50.0f);
    assert(positions[static_cast<std::size_t>(static_obj)]->y == 50.0f);

    std::cout << "✓ Movement system works correctly" << std::endl;
    std::cout << "  Player moved to (" << positions[static_cast<std::size_t>(player)]->x << ", " << positions[static_cast<std::size_t>(player)]->y << ")" << std::endl;
    std::cout << "  Static object stayed at (" << positions[static_cast<std::size_t>(static_obj)]->x << ", " << positions[static_cast<std::size_t>(static_obj)]->y << ")" << std::endl;
}

void test_auto_register() {
    std::cout << "\n=== Test auto-registration ===" << std::endl;

    registry reg;

    auto entity = reg.spawn_entity();

    auto& positions = reg.get_components<Position>();

    reg.add_component(entity, Position(42.0f, 24.0f));

    assert(reg.has_component<Position>(entity));
    assert(positions[static_cast<std::size_t>(entity)]->x == 42.0f);

    std::cout << "✓ Auto-registration on get_components works" << std::endl;
}

int main() {
    std::cout << "Testing registry implementation...\n" << std::endl;

    try {
        test_spawn_entities();
        test_register_and_get_components();
        test_add_component();
        test_emplace_component();
        test_has_component();
        test_remove_component();
        test_kill_entity();
        test_ecs_system();
        test_auto_register();

        std::cout << "\n✅ All registry tests passed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\n❌ Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
