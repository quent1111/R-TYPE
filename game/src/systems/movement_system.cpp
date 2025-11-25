#include "systems/movement_system.hpp"
#include "ecs/components.hpp"
#include "components/game_components.hpp"

void movementSystem(registry& reg, float dt) {
    auto& positions = reg.get_components<position>();
    auto& velocities = reg.get_components<velocity>();

    for (std::size_t i = 0; i < positions.size() && i < velocities.size(); ++i) {
        auto& pos_opt = positions[i];
        auto& vel_opt = velocities[i];

        if (pos_opt && vel_opt) {
            auto& pos = pos_opt.value();
            auto& vel = vel_opt.value();

            pos.x += vel.vx * dt;
            pos.y += vel.vy * dt;
        }
    }

    auto& bounds = reg.get_components<bounded_movement>();
    for (std::size_t i = 0; i < positions.size() && i < bounds.size(); ++i) {
        auto& pos_opt = positions[i];
        auto& bound_opt = bounds[i];

        if (pos_opt && bound_opt) {
            auto& pos = pos_opt.value();
            auto& bound = bound_opt.value();

            if (pos.x < bound.min_x) pos.x = bound.min_x;
            if (pos.x > bound.max_x) pos.x = bound.max_x;
            if (pos.y < bound.min_y) pos.y = bound.min_y;
            if (pos.y > bound.max_y) pos.y = bound.max_y;
        }
    }
}
