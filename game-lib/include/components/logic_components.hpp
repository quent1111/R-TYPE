#pragma once

#include "../../../src/Common/Opcodes.hpp"

// Composants de Gameplay Logique (Server-Safe, pas de SFML)

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

struct controllable {
    float speed;
    constexpr explicit controllable(float move_speed = 200.0f) noexcept : speed(move_speed) {}
};

struct weapon {
    float fire_rate;
    float time_since_shot;
    float projectile_speed;
    int damage;

    constexpr weapon(float rate = 5.0f, float proj_speed = 500.0f, int dmg = 10) noexcept
        : fire_rate(rate), time_since_shot(0.0f), projectile_speed(proj_speed), damage(dmg) {}

    constexpr void update(float dt) noexcept { time_since_shot += dt; }

    [[nodiscard]] constexpr bool can_shoot() const noexcept {
        return time_since_shot >= (1.0f / fire_rate);
    }

    constexpr void reset_shot_timer() noexcept { time_since_shot = 0.0f; }
};

struct bounded_movement {
    float min_x, max_x;
    float min_y, max_y;

    constexpr bounded_movement(float minx = 0.0f, float maxx = 1920.0f, float miny = 0.0f,
                                float maxy = 1080.0f) noexcept
        : min_x(minx), max_x(maxx), min_y(miny), max_y(maxy) {}
};

struct wave_manager {
    float timer = 0.0f;
    float spawn_interval = 5.0f;
    int enemies_per_wave = 3;
    
    constexpr wave_manager(float interval = 5.0f, int count = 3) noexcept 
        : spawn_interval(interval), enemies_per_wave(count) {}
};

// Tags
struct player_tag {};
struct enemy_tag {};
struct projectile_tag {};
struct explosion_tag {
    float lifetime;
    float elapsed;

    constexpr explosion_tag(float life = 0.5f) noexcept
        : lifetime(life), elapsed(0.0f) {}
};

struct entity_tag {
    RType::EntityType type;
    constexpr explicit entity_tag(RType::EntityType t_type) noexcept : type(t_type) {}
};

struct network_id {
    int client_id;
    constexpr explicit network_id(int c_id = -1) noexcept : client_id(c_id) {}
};
