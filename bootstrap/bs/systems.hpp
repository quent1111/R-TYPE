#pragma once

#include "registry.hpp"
#include "components.hpp"
#include <SFML/Graphics.hpp>
#include <iostream>
#include <cmath>

// SYSTEM 1: position_system
// Updates position based on velocity for all entities having both components
void position_system(registry& reg, float dt)
{
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

// SYSTEM 2: control_system
// Sets velocity based on keyboard input for controllable entities
void control_system(registry& reg)
{
    auto& velocities = reg.get_components<velocity>();
    auto& controllables = reg.get_components<controllable>();

    // Iterate through all entities
    for (std::size_t i = 0; i < velocities.size() && i < controllables.size(); ++i) {
        auto& vel_opt = velocities[i];
        auto& ctrl_opt = controllables[i];

        // Only process entities that have both components
        if (vel_opt && ctrl_opt) {
            auto& vel = vel_opt.value();
            auto& ctrl = ctrl_opt.value();

            // Reset velocity to 0
            vel.vx = 0.0f;
            vel.vy = 0.0f;

            // Set velocity based on keyboard input
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) || 
                sf::Keyboard::isKeyPressed(sf::Keyboard::Q)) {
                vel.vx = -ctrl.speed;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) || 
                sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
                vel.vx = ctrl.speed;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) || 
                sf::Keyboard::isKeyPressed(sf::Keyboard::Z)) {
                vel.vy = -ctrl.speed;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) || 
                sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
                vel.vy = ctrl.speed;
            }
        }
    }
}

// SYSTEM 3: draw_system
// Draws all entities with drawable and position components
void draw_system(registry& reg, sf::RenderWindow& window)
{
    auto& positions = reg.get_components<position>();
    auto& drawables = reg.get_components<drawable>();

    // Iterate through all entities
    for (std::size_t i = 0; i < positions.size() && i < drawables.size(); ++i) {
        auto& pos_opt = positions[i];
        auto& draw_opt = drawables[i];

        // Only process entities that have both components
        if (pos_opt && draw_opt) {
            auto& pos = pos_opt.value();
            auto& draw = draw_opt.value();

            // Set shape position and draw it
            draw.shape.setPosition(pos.x, pos.y);
            window.draw(draw.shape);
        }
    }
}

// EXAMPLE: logging_system (from subject example)
// Logs position and velocity to stderr for debugging
void logging_system(registry& reg)
{
    auto& positions = reg.get_components<position>();
    auto& velocities = reg.get_components<velocity>();

    for (std::size_t i = 0; i < positions.size() && i < velocities.size(); ++i) {
        auto& pos_opt = positions[i];
        auto& vel_opt = velocities[i];

        if (pos_opt && vel_opt) {
            auto& pos = pos_opt.value();
            auto& vel = vel_opt.value();

            std::cerr << "Entity " << i 
                      << " - Position: (" << pos.x << ", " << pos.y << ")"
                      << " - Velocity: (" << vel.vx << ", " << vel.vy << ")"
                      << std::endl;
        }
    }
}

// SYSTEM 4: acceleration_control_system
// Applies acceleration based on keyboard input for entities with controllable + acceleration + velocity
void acceleration_control_system(registry& reg, float dt)
{
    auto& velocities = reg.get_components<velocity>();
    auto& controllables = reg.get_components<controllable>();
    auto& accelerations = reg.get_components<acceleration>();

    for (std::size_t i = 0; i < velocities.size() && i < controllables.size() && i < accelerations.size(); ++i) {
        auto& vel_opt = velocities[i];
        auto& ctrl_opt = controllables[i];
        auto& acc_opt = accelerations[i];

        if (vel_opt && ctrl_opt && acc_opt) {
            auto& vel = vel_opt.value();
            auto& ctrl = ctrl_opt.value();
            auto& acc = acc_opt.value();

            // Reset acceleration
            acc.ax = 0.0f;
            acc.ay = 0.0f;

            // Set acceleration based on keyboard input
            float accel_amount = ctrl.speed;  // Use speed as acceleration rate
            
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left) || 
                sf::Keyboard::isKeyPressed(sf::Keyboard::Q)) {
                acc.ax = -accel_amount;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right) || 
                sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
                acc.ax = accel_amount;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up) || 
                sf::Keyboard::isKeyPressed(sf::Keyboard::Z)) {
                acc.ay = -accel_amount;
            }
            if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down) || 
                sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
                acc.ay = accel_amount;
            }

            // Apply acceleration to velocity
            vel.vx += acc.ax * dt;
            vel.vy += acc.ay * dt;

            // Apply friction (deceleration when no input)
            if (acc.ax == 0.0f) {
                vel.vx *= acc.friction;
            }
            if (acc.ay == 0.0f) {
                vel.vy *= acc.friction;
            }

            // Clamp to max speed
            float speed = std::sqrt(vel.vx * vel.vx + vel.vy * vel.vy);
            if (speed > acc.max_speed) {
                float scale = acc.max_speed / speed;
                vel.vx *= scale;
                vel.vy *= scale;
            }
        }
    }
}

// SYSTEM 5: looping_system
// Makes entities wrap around screen boundaries
void looping_system(registry& reg)
{
    auto& positions = reg.get_components<position>();
    auto& loopings = reg.get_components<looping>();
    auto& drawables = reg.get_components<drawable>();

    for (std::size_t i = 0; i < positions.size() && i < loopings.size(); ++i) {
        auto& pos_opt = positions[i];
        auto& loop_opt = loopings[i];

        if (pos_opt && loop_opt) {
            auto& pos = pos_opt.value();
            auto& loop = loop_opt.value();

            // Get entity size (default to 50x50 if no drawable component)
            float entity_width = 50.0f;
            float entity_height = 50.0f;
            
            if (i < drawables.size() && drawables[i]) {
                auto& draw = drawables[i].value();
                sf::Vector2f size = draw.shape.getSize();
                entity_width = size.x;
                entity_height = size.y;
            }

            // Wrap horizontally
            if (pos.x + entity_width < 0) {
                pos.x = loop.screen_width;
            } else if (pos.x > loop.screen_width) {
                pos.x = -entity_width;
            }

            // Wrap vertically
            if (pos.y + entity_height < 0) {
                pos.y = loop.screen_height;
            } else if (pos.y > loop.screen_height) {
                pos.y = -entity_height;
            }
        }
    }
}
