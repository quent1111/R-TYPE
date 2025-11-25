#pragma once

// Server-side components (no SFML dependencies)

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

// Component: Health for entities
struct health {
    int current;
    int maximum;

    constexpr health(int max = 100) noexcept : current(max), maximum(max) {}
    constexpr health(int current, int max) noexcept : current(current), maximum(max) {}
};

// Component: Damage dealing capability
struct damage {
    int amount;

    constexpr explicit damage(int amount = 10) noexcept : amount(amount) {}
};

// Component: Collision box for physics
struct collider {
    float width;
    float height;

    constexpr collider(float w = 32.0f, float h = 32.0f) noexcept : width(w), height(h) {}
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

    constexpr explicit entity_tag(entity_type type) noexcept : type(type) {}
};

// Component: Network client ID (for multiplayer)
struct network_id {
    int client_id;

    constexpr explicit network_id(int id = -1) noexcept : client_id(id) {}
};
