#include "systems/movement_system.hpp"
#include "ecs/components.hpp"
#include "components/game_components.hpp"
#include <cmath>

void movementSystem(registry& reg, float dt) {
    auto& positions = reg.get_components<position>();
    auto& velocities = reg.get_components<velocity>();
    auto& entity_tags = reg.get_components<entity_tag>();

    static float wave_time = 0.0f;
    wave_time += dt;
    static float zigzag_timer = 0.0f;
    zigzag_timer += dt;

    for (std::size_t i = 0; i < positions.size() && i < velocities.size(); ++i) {
        auto& pos_opt = positions[i];
        auto& vel_opt = velocities[i];

        if (pos_opt && vel_opt) {
            auto& pos = pos_opt.value();
            auto& vel = vel_opt.value();

            if (i < entity_tags.size() && entity_tags[i].has_value() &&
                entity_tags[i]->type == RType::EntityType::Enemy4) {
                pos.x += vel.vx * dt;
                pos.y += std::sin(wave_time * 3.0f + pos.x * 0.01f) * 100.0f * dt;
            }
            else if (i < entity_tags.size() && entity_tags[i].has_value() &&
                     entity_tags[i]->type == RType::EntityType::Enemy5) {
                pos.x += vel.vx * dt;
                float zigzag_phase = std::fmod(zigzag_timer + static_cast<float>(i) * 0.5f, 2.0f);
                float zigzag_direction = (zigzag_phase < 1.0f) ? 1.0f : -1.0f;
                pos.y += zigzag_direction * 150.0f * dt;
            } else {
                pos.x += vel.vx * dt;
                pos.y += vel.vy * dt;
            }
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
