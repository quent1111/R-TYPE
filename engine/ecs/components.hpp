#pragma once

// Base ECS Components
// These are generic, reusable components that can be used in any game

// Component: 2D position of an entity
struct position {
    float x;
    float y;

    constexpr position(float x = 0.0f, float y = 0.0f) noexcept : x(x), y(y) {}
};

// Component: 2D velocity of an entity
struct velocity {
    float vx;
    float vy;

    constexpr velocity(float vx = 0.0f, float vy = 0.0f) noexcept : vx(vx), vy(vy) {}
};
