#pragma once

// Base ECS Components
// These are generic, reusable components that can be used in any game

struct position {
    float x;
    float y;

    constexpr position(float pos_x = 0.0f, float pos_y = 0.0f) noexcept : x(pos_x), y(pos_y) {}
};

struct velocity {
    float vx;
    float vy;

    constexpr velocity(float vel_x = 0.0f, float vel_y = 0.0f) noexcept : vx(vel_x), vy(vel_y) {}
};
