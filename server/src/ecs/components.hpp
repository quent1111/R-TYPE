#pragma once

#include "../../../src/Common/Opcodes.hpp"

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

struct health {
    int current;
    int maximum;

    constexpr health(int max_hp = 100) noexcept : current(max_hp), maximum(max_hp) {}
    constexpr health(int curr_hp, int max_hp) noexcept : current(curr_hp), maximum(max_hp) {}
};

struct damage {
    int amount;

    constexpr explicit damage(int dmg_amount = 10) noexcept : amount(dmg_amount) {}
};

struct collider {
    float width;
    float height;

    constexpr collider(float w_val = 32.0f, float h_val = 32.0f) noexcept
        : width(w_val), height(h_val) {}
};

struct entity_tag {
    RType::EntityType type;

    constexpr explicit entity_tag(RType::EntityType t_type) noexcept : type(t_type) {}
};

struct network_id {
    int client_id;

    constexpr explicit network_id(int c_id = -1) noexcept : client_id(c_id) {}
};
