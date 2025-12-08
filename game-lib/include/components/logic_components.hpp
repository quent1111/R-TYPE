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

// Types d'améliorations d'armes
enum class WeaponUpgradeType : uint8_t {
    None = 0,
    PowerShot = 1,    // Tirs plus puissants (balles plus grosses, plus de dégâts)
    TripleShot = 2    // 3 projectiles (2 en diagonale + 1 droit)
};

struct weapon {
    float fire_rate;
    float time_since_shot;
    float projectile_speed;
    int damage;
    WeaponUpgradeType upgrade_type;

    constexpr weapon(float rate = 5.0f, float proj_speed = 500.0f, int dmg = 10, 
                     WeaponUpgradeType upgrade = WeaponUpgradeType::None) noexcept
        : fire_rate(rate), time_since_shot(0.0f), projectile_speed(proj_speed), 
          damage(dmg), upgrade_type(upgrade) {}

    constexpr void update(float dt) noexcept { time_since_shot += dt; }

    [[nodiscard]] constexpr bool can_shoot() const noexcept {
        return time_since_shot >= (1.0f / fire_rate);
    }

    constexpr void reset_shot_timer() noexcept { time_since_shot = 0.0f; }
    
    constexpr void apply_upgrade(WeaponUpgradeType new_upgrade) noexcept {
        upgrade_type = new_upgrade;
        if (new_upgrade == WeaponUpgradeType::PowerShot) {
            damage = 25;  // Plus de dégâts
        } else if (new_upgrade == WeaponUpgradeType::TripleShot) {
            fire_rate = 4.0f;  // Légèrement moins rapide pour équilibrer
        }
    }
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

// Gestionnaire de niveaux
struct level_manager {
    int current_level = 1;
    int enemies_killed_this_level = 0;
    int enemies_needed_for_next_level = 20;
    bool awaiting_upgrade_choice = false;
    bool level_completed = false;
    float level_start_delay = 3.0f;  // Délai de 3 secondes au début de chaque niveau
    float level_start_timer = 0.0f;
    
    constexpr level_manager() noexcept = default;
    
    constexpr void on_enemy_killed() noexcept {
        enemies_killed_this_level++;
        if (enemies_killed_this_level == enemies_needed_for_next_level) {
            level_completed = true;
            awaiting_upgrade_choice = true;
        }
    }
    
    constexpr void advance_to_next_level() noexcept {
        current_level++;
        enemies_killed_this_level = 0;
        level_completed = false;
        awaiting_upgrade_choice = false;
        // Augmente la difficulté
        enemies_needed_for_next_level = 20 + (current_level - 1) * 10;
        // Réinitialiser le timer de délai
        level_start_timer = 0.0f;
    }
    
    [[nodiscard]] constexpr int get_progress_percentage() const noexcept {
        if (enemies_needed_for_next_level == 0) return 0;
        return (enemies_killed_this_level * 100) / enemies_needed_for_next_level;
    }
    
    [[nodiscard]] constexpr bool is_level_intro_active() const noexcept {
        return level_start_timer < level_start_delay;
    }
    
    constexpr void update_intro_timer(float dt) noexcept {
        if (level_start_timer < level_start_delay) {
            level_start_timer += dt;
        }
    }
};

// Power-ups
enum class PowerUpType : uint8_t {
    None = 0,
    PowerCannon = 1,
    Shield = 2
};

struct power_cannon {
    bool active = false;
    float duration = 10.0f;
    float time_remaining = 0.0f;
    int damage = 50;  // 5x damage normal
    float fire_rate = 3.0f;  // Plus lent mais plus puissant
    
    constexpr power_cannon() noexcept = default;
    
    constexpr void activate() noexcept {
        active = true;
        time_remaining = duration;
    }
    
    constexpr void update(float dt) noexcept {
        if (active) {
            time_remaining -= dt;
            if (time_remaining <= 0.0f) {
                active = false;
                time_remaining = 0.0f;
            }
        }
    }
    
    [[nodiscard]] constexpr bool is_active() const noexcept { return active; }
    [[nodiscard]] constexpr float get_remaining_percentage() const noexcept {
        return duration > 0.0f ? (time_remaining / duration) : 0.0f;
    }
};

struct shield {
    bool active = false;
    float duration = 10.0f;
    float time_remaining = 0.0f;
    float radius = 80.0f;  // Rayon du bouclier autour du joueur
    
    constexpr shield() noexcept = default;
    
    constexpr void activate() noexcept {
        active = true;
        time_remaining = duration;
    }
    
    constexpr void update(float dt) noexcept {
        if (active) {
            time_remaining -= dt;
            if (time_remaining <= 0.0f) {
                active = false;
                time_remaining = 0.0f;
            }
        }
    }
    
    [[nodiscard]] constexpr bool is_active() const noexcept { return active; }
    [[nodiscard]] constexpr float get_remaining_percentage() const noexcept {
        return duration > 0.0f ? (time_remaining / duration) : 0.0f;
    }
    
    [[nodiscard]] constexpr bool is_enemy_in_range(float enemy_x, float enemy_y, 
                                                     float player_x, float player_y) const noexcept {
        if (!active) return false;
        float dx = enemy_x - player_x;
        float dy = enemy_y - player_y;
        float dist_squared = dx * dx + dy * dy;
        return dist_squared <= (radius * radius);
    }
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
