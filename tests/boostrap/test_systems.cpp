#include "../../bootstrap/bs/registry.hpp"
#include "../../bootstrap/bs/components.hpp"
#include <iostream>
#include <cassert>
#include <cmath>

// =================== TEST SYSTEMS ===================

void logging_system(registry& r,
                   sparse_array<position> const& positions,
                   sparse_array<velocity> const& velocities)
{
    std::cout << "\n=== Logging System ===" << std::endl;
    for (size_t i = 0; i < positions.size() && i < velocities.size(); ++i) {
        auto const& pos = positions[i];
        auto const& vel = velocities[i];
        if (pos && vel) {
            std::cout << "Entity " << i << ": Position = { " << pos.value().x << ", "
                     << pos.value().y << " }, Velocity = { " << vel.value().vx << ", "
                     << vel.value().vy << " }" << std::endl;
        }
    }
}

void movement_system(registry& r,
                    sparse_array<position>& positions,
                    sparse_array<velocity> const& velocities)
{
    for (size_t i = 0; i < positions.size() && i < velocities.size(); ++i) {
        auto& pos = positions[i];
        auto const& vel = velocities[i];
        if (pos && vel) {
            pos.value().x += vel.value().vx;
            pos.value().y += vel.value().vy;
        }
    }
}

void boundary_system(registry& r,
                    sparse_array<position>& positions,
                    sparse_array<looping> const& loop_components)
{
    for (size_t i = 0; i < positions.size() && i < loop_components.size(); ++i) {
        auto& pos = positions[i];
        auto const& loop = loop_components[i];
        if (pos && loop) {
            if (pos.value().x < 0)
                pos.value().x = loop.value().screen_width;
            if (pos.value().x > loop.value().screen_width)
                pos.value().x = 0;
            if (pos.value().y < 0)
                pos.value().y = loop.value().screen_height;
            if (pos.value().y > loop.value().screen_height)
                pos.value().y = 0;
        }
    }
}

auto acceleration_system = [](registry& r,
                              sparse_array<velocity>& velocities,
                              sparse_array<acceleration> const& accelerations) {
    for (size_t i = 0; i < velocities.size() && i < accelerations.size(); ++i) {
        auto& vel = velocities[i];
        auto const& acc = accelerations[i];
        if (vel && acc) {
            vel.value().vx += acc.value().ax;
            vel.value().vy += acc.value().ay;
            
            vel.value().vx *= acc.value().friction;
            vel.value().vy *= acc.value().friction;
            
            float speed = std::sqrt(vel.value().vx * vel.value().vx + 
                                   vel.value().vy * vel.value().vy);
            if (speed > acc.value().max_speed) {
                float scale = acc.value().max_speed / speed;
                vel.value().vx *= scale;
                vel.value().vy *= scale;
            }
        }
    }
};

// =================== TESTS ===================

void test_basic_systems() {
    std::cout << "=== Test: Basic System Registration ===" << std::endl;
    
    registry reg;
    
    reg.register_component<position>();
    reg.register_component<velocity>();
    
    auto e1 = reg.spawn_entity();
    auto e2 = reg.spawn_entity();
    auto e3 = reg.spawn_entity();
    
    reg.emplace_component<position>(e1, 0.0f, 0.0f);
    reg.emplace_component<velocity>(e1, 10.0f, 5.0f);
    
    reg.emplace_component<position>(e2, 100.0f, 50.0f);
    reg.emplace_component<velocity>(e2, -5.0f, 10.0f);
    
    reg.emplace_component<position>(e3, 200.0f, 150.0f);
    
    reg.add_system<position, velocity>(logging_system);
    reg.add_system<position, velocity>(movement_system);
    
    std::cout << "\n--- Before movement ---" << std::endl;
    reg.run_systems();
    
    std::cout << "\n--- After movement ---" << std::endl;
    reg.add_system<position, velocity>(logging_system);
    reg.run_systems();
    
    auto& positions = reg.get_components<position>();
    assert(positions[static_cast<std::size_t>(e1)]->x == 20.0f);
    assert(positions[static_cast<std::size_t>(e1)]->y == 10.0f);
    
    std::cout << "\n✓ Basic systems work correctly" << std::endl;
}

void test_lambda_systems() {
    std::cout << "\n=== Test: Lambda Systems ===" << std::endl;
    
    registry reg;
    
    auto e1 = reg.spawn_entity();
    reg.emplace_component<velocity>(e1, 5.0f, 5.0f);
    reg.emplace_component<acceleration>(e1, 1.0f, 0.5f, 100.0f, 0.95f);
    
    reg.add_system<velocity, acceleration>(acceleration_system);
    
    auto& velocities = reg.get_components<velocity>();
    
    std::cout << "Before acceleration: vx=" << velocities[static_cast<std::size_t>(e1)]->vx
              << ", vy=" << velocities[static_cast<std::size_t>(e1)]->vy << std::endl;
    
    reg.run_systems();
    
    std::cout << "After acceleration: vx=" << velocities[static_cast<std::size_t>(e1)]->vx
              << ", vy=" << velocities[static_cast<std::size_t>(e1)]->vy << std::endl;
    
    assert(velocities[static_cast<std::size_t>(e1)]->vx > 5.0f);
    assert(velocities[static_cast<std::size_t>(e1)]->vy > 5.0f);
    
    std::cout << "✓ Lambda systems work correctly" << std::endl;
}

void test_boundary_system() {
    std::cout << "\n=== Test: Boundary Wrapping System ===" << std::endl;
    
    registry reg;
    
    auto e1 = reg.spawn_entity();
    reg.emplace_component<position>(e1, 850.0f, 100.0f);
    reg.emplace_component<looping>(e1, 800.0f, 600.0f);
    
    reg.add_system<position, looping>(boundary_system);
    
    auto& positions = reg.get_components<position>();
    
    std::cout << "Before boundary: x=" << positions[static_cast<std::size_t>(e1)]->x << std::endl;
    
    reg.run_systems();
    
    std::cout << "After boundary: x=" << positions[static_cast<std::size_t>(e1)]->x << std::endl;
    
    assert(positions[static_cast<std::size_t>(e1)]->x == 0.0f);
    
    std::cout << "✓ Boundary system works correctly" << std::endl;
}

void test_inline_lambda_system() {
    std::cout << "\n=== Test: Inline Lambda System ===" << std::endl;
    
    registry reg;
    
    auto e1 = reg.spawn_entity();
    reg.emplace_component<position>(e1, 10.0f, 20.0f);
    
    reg.add_system<position>([](registry& r, sparse_array<position>& positions) {
        std::cout << "Inline lambda system running..." << std::endl;
        for (size_t i = 0; i < positions.size(); ++i) {
            if (positions[i]) {
                std::cout << "  Entity " << i << " at (" << positions[i]->x
                         << ", " << positions[i]->y << ")" << std::endl;
                positions[i]->x *= 2;
                positions[i]->y *= 2;
            }
        }
    });
    
    reg.run_systems();
    
    auto& positions = reg.get_components<position>();
    assert(positions[static_cast<std::size_t>(e1)]->x == 20.0f);
    assert(positions[static_cast<std::size_t>(e1)]->y == 40.0f);
    
    std::cout << "✓ Inline lambda system works correctly" << std::endl;
}

void test_multiple_systems() {
    std::cout << "\n=== Test: Multiple Systems in Order ===" << std::endl;
    
    registry reg;
    
    auto player = reg.spawn_entity();
    reg.emplace_component<position>(player, 400.0f, 300.0f);
    reg.emplace_component<velocity>(player, 5.0f, 0.0f);
    reg.emplace_component<looping>(player, 800.0f, 600.0f);
    
    reg.add_system<position, velocity>(movement_system);
    reg.add_system<position, looping>(boundary_system);
    reg.add_system<position, velocity>(logging_system);
    
    std::cout << "\nRunning all systems in order..." << std::endl;
    reg.run_systems();
    
    std::cout << "\n✓ Multiple systems execute in registration order" << std::endl;
}

int main() {
    std::cout << "Testing ECS Systems in Registry...\n" << std::endl;
    
    try {
        test_basic_systems();
        test_lambda_systems();
        test_boundary_system();
        test_inline_lambda_system();
        test_multiple_systems();
        
        std::cout << "\n✅ All system tests passed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "\n❌ Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
