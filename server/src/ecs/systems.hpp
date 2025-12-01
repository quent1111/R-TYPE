#pragma once

#include "components.hpp"
#include "registry.hpp"

#include <cmath>

#include <algorithm>
#include <iostream>

// SYSTEM 1: MOVEMENT
inline void position_system(registry& reg, float dt) {
    auto& positions = reg.get_components<position>();
    auto& velocities = reg.get_components<velocity>();

    size_t limit = std::min(positions.size(), velocities.size());

    for (std::size_t i = 0; i < limit; ++i) {
        auto& pos_opt = positions[i];
        auto& vel_opt = velocities[i];

        if (pos_opt && vel_opt) {
            auto& pos = pos_opt.value();
            auto& vel = vel_opt.value();

            pos.x += vel.vx * dt;
            pos.y += vel.vy * dt;
        }
    }
}

// SYSTEM 2: COLLISION
inline void collision_system(registry& reg) {
    auto& positions = reg.get_components<position>();
    auto& colliders = reg.get_components<collider>();

    size_t limit = std::min(positions.size(), colliders.size());

    for (std::size_t i = 0; i < limit; ++i) {
        if (!positions[i] || !colliders[i])
            continue;

        auto& pos1 = positions[i].value();
        auto& col1 = colliders[i].value();

        for (std::size_t j = i + 1; j < limit; ++j) {
            if (!positions[j] || !colliders[j])
                continue;

            auto& pos2 = positions[j].value();
            auto& col2 = colliders[j].value();

            if (pos1.x < pos2.x + col2.width && pos1.x + col1.width > pos2.x &&
                pos1.y < pos2.y + col2.height && pos1.y + col1.height > pos2.y) {
                std::cout << "[Physics] Collision: Entity " << i << " <-> " << j << std::endl;
                // Logique de collision (recul, etc.) à ajouter ici
            }
        }
    }
}

// SYSTEM 3: DAMAGE
inline void damage_system(registry& reg) {
    auto& positions = reg.get_components<position>();
    auto& colliders = reg.get_components<collider>();
    auto& healths = reg.get_components<health>();
    auto& damages = reg.get_components<damage>();

    size_t limit = positions.size();
    if (colliders.size() < limit)
        limit = colliders.size();
    if (damages.size() < limit)
        limit = damages.size();

    for (std::size_t i = 0; i < limit; ++i) {
        if (!positions[i] || !colliders[i] || !damages[i])
            continue;

        auto& pos1 = positions[i].value();
        auto& col1 = colliders[i].value();
        auto& dmg = damages[i].value();

        size_t target_limit = std::min({positions.size(), colliders.size(), healths.size()});

        for (std::size_t j = 0; j < target_limit; ++j) {
            if (i == j)
                continue;

            if (!positions[j] || !colliders[j] || !healths[j])
                continue;

            auto& pos2 = positions[j].value();
            auto& col2 = colliders[j].value();
            auto& hp = healths[j].value();

            if (pos1.x < pos2.x + col2.width && pos1.x + col1.width > pos2.x &&
                pos1.y < pos2.y + col2.height && pos1.y + col1.height > pos2.y) {
                hp.current -= dmg.amount;
            }
        }
    }
}

// SYSTEM 4: CLEANUP (Mort)
inline void cleanup_system(registry& reg) {
    auto& healths = reg.get_components<health>();

    for (std::size_t i = 0; i < healths.size(); ++i) {
        if (healths[i] && healths[i]->current <= 0) {
            reg.kill_entity(reg.entity_from_index(i));
            std::cout << "[Game] Entity " << i << " died." << std::endl;
        }
    }
}

// SYSTEM 5: BOUNDARIES (Sortie de map)
inline void boundary_system(registry& reg, float world_width, float world_height) {
    auto& positions = reg.get_components<position>();

    for (std::size_t i = 0; i < positions.size(); ++i) {
        if (positions[i]) {
            auto& pos = positions[i].value();

            if (pos.x < -100 || pos.x > world_width + 100 || pos.y < -100 ||
                pos.y > world_height + 100) {
                reg.kill_entity(reg.entity_from_index(i));
            }
        }
    }
}

// UTILS: LOGGING
inline void logging_system(registry& reg) {
    auto& positions = reg.get_components<position>();
    auto& velocities = reg.get_components<velocity>();

    size_t limit = std::min(positions.size(), velocities.size());

    for (std::size_t i = 0; i < limit; ++i) {
        if (positions[i] && velocities[i]) {
            auto& pos = positions[i].value();
            auto& vel = velocities[i].value();
            std::cerr << "Entity " << i << ": Pos(" << pos.x << ", " << pos.y << ") Vel(" << vel.vx
                      << ", " << vel.vy << ")" << std::endl;
        }
    }
}
