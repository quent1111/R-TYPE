#pragma once

// Server-side components (no SFML dependencies)

// Component: 2D position of an entity
struct position {
    float x;
    float y;

    // x -> p_x, y -> p_y
    constexpr position(float p_x = 0.0f, float p_y = 0.0f) noexcept : x(p_x), y(p_y) {}
};

// Component: 2D velocity of an entity
struct velocity {
    float vx;
    float vy;

    // vx -> p_vx, vy -> p_vy
    constexpr velocity(float p_vx = 0.0f, float p_vy = 0.0f) noexcept : vx(p_vx), vy(p_vy) {}
};

// Component: Health for entities
struct health {
    int current;
    int maximum;

    // max -> max_hp, current -> curr_hp
    constexpr health(int max_hp = 100) noexcept : current(max_hp), maximum(max_hp) {}
    constexpr health(int curr_hp, int max_hp) noexcept : current(curr_hp), maximum(max_hp) {}
};

// Component: Damage dealing capability
struct damage {
    int amount;

    // amount -> dmg_amount
    constexpr explicit damage(int dmg_amount = 10) noexcept : amount(dmg_amount) {}
};

// Component: Collision box for physics
struct collider {
    float width;
    float height;

    // w -> w_val, h -> h_val
    constexpr collider(float w_val = 32.0f, float h_val = 32.0f) noexcept : width(w_val), height(h_val) {}
};

// Component: Entity type identifier
enum class entity_type {
    PLAYER,
    ENEMY,
    PROJECTILE,
    POWERUP
};

struct entity_tag {
    entity_type type;

    // type -> t_type
    constexpr explicit entity_tag(entity_type t_type) noexcept : type(t_type) {}
};

// Component: Network client ID (for multiplayer)
struct network_id {
    int client_id;

    // id -> c_id
    constexpr explicit network_id(int c_id = -1) noexcept : client_id(c_id) {}
};
