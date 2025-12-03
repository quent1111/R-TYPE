#pragma once

struct position {
    float x;
    float y;

    constexpr position(float p_x = 0.0f, float p_y = 0.0f) noexcept : x(p_x), y(p_y) {}
};

struct velocity {
    float vx;
    float vy;

    constexpr velocity(float p_vx = 0.0f, float p_vy = 0.0f) noexcept : vx(p_vx), vy(p_vy) {}
};

struct collider {
    float width;
    float height;

    constexpr collider(float w_val = 32.0f, float h_val = 32.0f) noexcept
        : width(w_val), height(h_val) {}
};
