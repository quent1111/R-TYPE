#pragma once

#include "../../src/Common/Opcodes.hpp"

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

    [[nodiscard]] constexpr bool is_alive() const noexcept { return current > 0; }
    [[nodiscard]] constexpr bool is_dead() const noexcept { return current <= 0; }
    [[nodiscard]] constexpr float health_percentage() const noexcept {
        return maximum > 0 ? static_cast<float>(current) / static_cast<float>(maximum) : 0.0f;
    }
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


struct damage_on_contact {
    int damage_amount;
    bool destroy_on_hit;

    constexpr damage_on_contact(int dmg = 10, bool destroy = true) noexcept
        : damage_amount(dmg), destroy_on_hit(destroy) {}
};

struct collision_box {
    float width;
    float height;
    float offset_x;
    float offset_y;

    constexpr collision_box(float w = 50.0f, float h = 50.0f, float ox = 0.0f,
                             float oy = 0.0f) noexcept
        : width(w), height(h), offset_x(ox), offset_y(oy) {}
};

struct player_tag {};

struct enemy_tag {};

struct projectile_tag {};