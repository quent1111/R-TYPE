#pragma once

#include "components.hpp"
#include "registry.hpp"

#include <cmath>
#include <iostream>

// SYSTEM 1: position_system
// Updates position based on velocity for all entities having both components
inline void position_system(registry& reg, float dt) {
    auto& positions = reg.get_components<position>();
    auto& velocities = reg.get_components<velocity>();

    // Iterate through all entities
    for (std::size_t i = 0; i < positions.size() && i < velocities.size(); ++i) {
        auto& pos_opt = positions[i];
        auto& vel_opt = velocities[i];

        // Only process entities that have both components
        if (pos_opt && vel_opt) {
            auto& pos = pos_opt.value();
            auto& vel = vel_opt.value();

            // Add velocity to position (scaled by delta time)
            pos.x += vel.vx * dt;
            pos.y += vel.vy * dt;
        }
    }
}

// SYSTEM 2: collision_system
// Checks for collisions between entities
inline void collision_system(registry& reg) {
    auto& positions = reg.get_components<position>();
    auto& colliders = reg.get_components<collider>();

    // Simple AABB collision detection
    for (std::size_t i = 0; i < positions.size() && i < colliders.size(); ++i) {
        auto& pos1_opt = positions[i];
        auto& col1_opt = colliders[i];

        if (pos1_opt && col1_opt) {
            auto& pos1 = pos1_opt.value();
            auto& col1 = col1_opt.value();

            for (std::size_t j = i + 1; j < positions.size() && j < colliders.size(); ++j) {
                auto& pos2_opt = positions[j];
                auto& col2_opt = colliders[j];

                if (pos2_opt && col2_opt) {
                    auto& pos2 = pos2_opt.value();
                    auto& col2 = col2_opt.value();

                    // AABB collision check
                    bool collision = pos1.x < pos2.x + col2.width &&
                                     pos1.x + col1.width > pos2.x &&
                                     pos1.y < pos2.y + col2.height &&
                                     pos1.y + col1.height > pos2.y;

                    if (collision) {
                        std::cout << "Collision detected between entity " << i << " and " << j << std::endl;
                        // Handle collision here (you can add callbacks or damage system)
                    }
                }
            }
        }
    }
}

// SYSTEM 3: damage_system
// Applies damage when entities collide
inline void damage_system(registry& reg) {
    auto& positions = reg.get_components<position>();
    auto& colliders = reg.get_components<collider>();
    auto& healths = reg.get_components<health>();
    auto& damages = reg.get_components<damage>();

    for (std::size_t i = 0; i < positions.size(); ++i) {
        auto& pos1_opt = positions[i];
        auto& col1_opt = colliders[i];
        auto& dmg_opt = damages[i];

        if (pos1_opt && col1_opt && dmg_opt) {
            auto& pos1 = pos1_opt.value();
            auto& col1 = col1_opt.value();
            auto& dmg = dmg_opt.value();

            for (std::size_t j = 0; j < positions.size(); ++j) {
                if (i == j) continue;

                auto& pos2_opt = positions[j];
                auto& col2_opt = colliders[j];
                auto& hp_opt = healths[j];

                if (pos2_opt && col2_opt && hp_opt) {
                    auto& pos2 = pos2_opt.value();
                    auto& col2 = col2_opt.value();
                    auto& hp = hp_opt.value();

                    // Check collision
                    bool collision = pos1.x < pos2.x + col2.width &&
                                     pos1.x + col1.width > pos2.x &&
                                     pos1.y < pos2.y + col2.height &&
                                     pos1.y + col1.height > pos2.y;

                    if (collision) {
                        hp.current -= dmg.amount;
                        if (hp.current <= 0) {
                            std::cout << "Entity " << j << " destroyed!" << std::endl;
                        }
                    }
                }
            }
        }
    }
}

// EXAMPLE: logging_system (from subject example)
// Logs position and velocity to stderr for debugging
inline void logging_system(registry& reg) {
    auto& positions = reg.get_components<position>();
    auto& velocities = reg.get_components<velocity>();

    for (std::size_t i = 0; i < positions.size() && i < velocities.size(); ++i) {
        auto& pos_opt = positions[i];
        auto& vel_opt = velocities[i];

        if (pos_opt && vel_opt) {
            auto& pos = pos_opt.value();
            auto& vel = vel_opt.value();

            std::cerr << "Entity " << i << " - Position: (" << pos.x << ", " << pos.y << ")"
                      << " - Velocity: (" << vel.vx << ", " << vel.vy << ")" << std::endl;
        }
    }
}

// SYSTEM 4: cleanup_system
// Removes dead entities (health <= 0)
inline void cleanup_system(registry& reg) {
    auto& healths = reg.get_components<health>();

    for (std::size_t i = 0; i < healths.size(); ++i) {
        auto& hp_opt = healths[i];

        if (hp_opt && hp_opt.value().current <= 0) {
            // Kill the entity - use entity_from_index to create entity from index
            entity e = reg.entity_from_index(i);
            reg.kill_entity(e);
            std::cout << "Entity " << i << " removed from registry" << std::endl;
        }
    }
}

// SYSTEM 5: boundary_system
// Removes entities that go out of bounds (server world boundaries)
inline void boundary_system(registry& reg, float world_width, float world_height) {
    auto& positions = reg.get_components<position>();
    auto& colliders = reg.get_components<collider>();

    for (std::size_t i = 0; i < positions.size(); ++i) {
        auto& pos_opt = positions[i];

        if (pos_opt) {
            auto& pos = pos_opt.value();
            float entity_width = 0.0f;
            float entity_height = 0.0f;

            if (i < colliders.size() && colliders[i]) {
                entity_width = colliders[i].value().width;
                entity_height = colliders[i].value().height;
            }

            // Check if entity is completely out of bounds
            if (pos.x + entity_width < 0 || pos.x > world_width ||
                pos.y + entity_height < 0 || pos.y > world_height) {
                entity e = reg.entity_from_index(i);
                reg.kill_entity(e);
                std::cout << "Entity " << i << " removed (out of bounds)" << std::endl;
            }
        }
    }
}
